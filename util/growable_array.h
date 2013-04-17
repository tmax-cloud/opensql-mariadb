/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ifndef UTIL_GROWABLE_ARRAY_H
#define UTIL_GROWABLE_ARRAY_H
#ident "$Id: partitioned_counter.cc 46098 2012-07-24 21:58:41Z bkuszmaul $"
#ident "Copyright (c) 2007-2012 Tokutek Inc.  All rights reserved."
#ident "The technology is licensed by the Massachusetts Institute of Technology, Rutgers State University of New Jersey, and the Research Foundation of State University of New York at Stony Brook under United States of America Serial No. 11/760379 and to the patents and/or patent applications resulting from it."

//******************************************************************************
//
// Overview: A growable array is a little bit like std::vector except that
//  it doesn't have constructors (hence can be used in static constructs, since
//  the google style guide says no constructors), and it's a little simpler.
// Operations:
//   init and deinit (we don't have constructors and destructors).
//   fetch_unchecked to get values out.
//   store_unchecked to put values in.
//   push to add an element at the end
//   get_size to find out the size
//   get_memory_size to find out how much memory the data stucture is using.
//
//******************************************************************************

namespace toku {

template<typename T> class GrowableArray {
 public:
    void init (void)
    // Effect: Initialize the array to contain no elements.
    {
	m_array=NULL;
	m_size=0;
	m_size_limit=0;
    }

    void deinit (void)
    // Effect: Deinitialize the array (freeing any memory it uses, for example).
    {
	toku_free(m_array);
	m_array     =NULL;
	m_size      =0;
	m_size_limit=0;
    }

    T fetch_unchecked (size_t i) const
    // Effect: Fetch the ith element.  If i is out of range, the system asserts.
    {
	return m_array[i];
    }

    void store_unchecked (size_t i, T v)
    // Effect: Store v in the ith element.  If i is out of range, the system asserts.
    {
	paranoid_invariant(i<m_size);
	m_array[i]=v;
    }

    void push (T v)
    // Effect: Add v to the end of the array (increasing the size).  The amortized cost of this operation is constant.
    // Implementation hint:  Double the size of the array when it gets too big so that the amortized cost stays constant.
    {
	if (m_size>=m_size_limit) {
	    if (m_array==NULL) {
		m_size_limit=1;
	    } else {
		m_size_limit*=2;
	    }
	    XREALLOC_N(m_size_limit, m_array);
	}
	m_array[m_size++]=v;
    }

    size_t get_size (void) const
    // Effect: Return the number of elements in the array.
    {
	return m_size;
    }
    size_t memory_size(void) const
    // Effect: Return the size (in bytes) that the array occupies in memory.  This is really only an estimate.
    {
	return sizeof(*this)+sizeof(T)*m_size_limit;
    }

 private:
    T     *m_array;
    size_t m_size;
    size_t m_size_limit; // How much space is allocated in array.
};

}

#endif // UTIL_GROWABLE_ARRAY_H
