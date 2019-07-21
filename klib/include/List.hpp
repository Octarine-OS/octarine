/*
 * Copyright (c) 2018, Devin Nakamura
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LIST_HPP
#define LIST_HPP

//#include <assert.h>

template <typename T> struct ListHook {
	T* prev;
	T* next;
	ListHook() {
		prev = nullptr;
		next = nullptr;
	}
};

template <typename T, ListHook<T> T::*list_data> class ListIterator {
	T* elem;

  public:
	constexpr ListIterator() : elem(nullptr) {}

	ListIterator(T* start) : elem(start) {}

	ListIterator& operator++() {
		elem = (elem->*list_data).next;
		return *this;
	}

	bool operator==(ListIterator<T, list_data>& other) const {
		return elem == other.elem;
	}
	bool operator!=(ListIterator<T, list_data>& other) const {
		return elem != other.elem;
	}

	ListIterator& operator=(ListIterator<T, list_data> other) {
		elem = other.elem;
		return *this;
	}

	T* operator*() { return elem; }

	T* operator->() { return elem; }
};

template <typename T, ListHook<T> T::*list_data> class List {
	T* m_head;
	T* m_tail;

	void insert(T* elem) {

		// TODO maybe check head==NULL || tail == NULL
		// but that would make us slower
		if (m_head == nullptr) {

			(elem->*list_data).next = elem;
			(elem->*list_data).prev = elem;
			m_head = m_tail = elem;
		} else {

			(m_tail->*list_data).next = elem;
			(elem->*list_data).prev = m_tail;

			(elem->*list_data).next = m_head;
			(m_head->*list_data).prev = elem;
		}
	}

  public:
	typedef ListIterator<T, list_data> Iterator;
	constexpr List() : m_head(nullptr), m_tail(nullptr) {}

	void insert_head(T* elem) {
		insert(elem);
		m_head = elem;
		if (m_tail == nullptr) {
			m_tail = elem;
		}
	}

	void insert_tail(T* elem) {
		insert(elem);
		m_tail = elem;
		if (m_head == nullptr) {
			m_head = elem;
		}
	}

	T* remove_head() { remove(m_head); }

	void remove(T* elem) {
		T* next = (elem->*list_data).next;
		T* prev = (elem->*list_data).prev;
		// assert(prev != NULL);
		// assert(next != NULL);
		if (next == elem) {
			// assert(prev == elem);
			// assert(elem == m_head);
			// assert(elem == m_tail);
			m_head = m_tail = nullptr;
			// TODO do we want to set new values of next and prev?
			return;
		}
		(prev->*list_data).next = next;
		(next->*list_data).prev = prev;

		if (m_head == elem) {
			m_head = next;
		}
		if (m_tail == elem) {
			m_tail = prev;
		}
	}

	T* head() { return m_head; }

	T* tail() { return m_tail; }

	Iterator begin() { return Iterator(m_head); }

	Iterator end() { return Iterator(m_tail); }
};

#endif