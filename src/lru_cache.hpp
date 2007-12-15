// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2004-2006 Patrick Audley
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

/***************************************************************************
 *   Copyright (C) 2004-2006 by Patrick Audley                             *
 *   paudley@blackcat.ca                                                   *
 *   http://blackcat.ca                                                    *
 *                                                                         *
 ***************************************************************************/
/**
 * @file lru_cache.h Template cache with an LRU removal policy
 * @author Patrick Audley
 * @version 1.1
 * @date March 2006
 * @par
 * This cache is thread safe if compiled with the _REENTRANT defined.  It
 * uses the BOOST scientific computing library to provide the thread safety
 * mutexes.
 *
 * @note ERG: This code was orriginally downloaded from
 *       http://lifelines.blackcat.ca/lifeline/query.php/tag=LRU_CACHE
 *       with the note that it is GPLv2. I have gone ahead and added
 *       the GPLv2 lilcense block above.
 *
 *       The original package contained unit tests which were mostly
 *       made of macros and had some odd sort of global state,
 *       including set up of the next unit test...in the previous unit
 *       test; they were not ported over to the TUT system.
 */
#include <map>
#include <list>
#include <vector>
#ifdef _REENTRANT
#include <boost/thread/mutex.hpp>
/// If we are reentrant then use a BOOST scoped mutex where neccessary.
#define SCOPED_MUTEX  boost::mutex::scoped_lock lock(this->_mutex);
#else
/// If we aren't reentrant then don't do anything.
#define SCOPED_MUTEX
#endif

/**
 * @brief Template cache with an LRU removal policy.
 * @class LRUCache
 * @author Patrick Audley
 *
 * @par
 * This template creats a simple collection of key-value pairs that grows
 * until the size specified at construction is reached and then begins
 * discard the Least Recently Used element on each insertion.
 *
 */
template<class Key,class Data> class LRUCache {
	public:
		typedef std::list< std::pair< Key, Data > > List;         ///< Main cache storage typedef
		typedef typename List::iterator List_Iter;                ///< Main cache iterator
		typedef typename List::const_iterator List_cIter;         ///< Main cache iterator (const)
		typedef std::vector< Key > Key_List;                      ///< List of keys
		typedef typename Key_List::iterator Key_List_Iter;        ///< Main cache iterator
		typedef typename Key_List::const_iterator Key_List_cIter; ///< Main cache iterator (const)
		typedef std::map< Key, List_Iter > Map;                   ///< Index typedef
		typedef std::pair< Key, List_Iter > Pair;                 ///< Pair of Map elements
		typedef typename Map::iterator Map_Iter;			            ///< Index iterator
		typedef typename Map::const_iterator Map_cIter;           ///< Index iterator (const)

	private:
		/// Main cache storage
		List _list;
		/// Cache storage index
		Map _index;

		/// Maximum size of the cache in elements
		unsigned long _max_size;

#ifdef _REENTRANT
		boost::mutex _mutex;
#endif

	public:
		/** @brief Creates a cache that holds at most Size elements.
		 *  @param Size maximum elements for cache to hold
		 */
		LRUCache( const unsigned long Size ) :
				_max_size( Size )
				{}
		/// Destructor - cleans up both index and storage
		~LRUCache() { this->clear(); }

		/** @brief Gets the current size (in elements) of the cache.
		 *  @return size in elements
		 */
		inline const unsigned long size( void ) {
			SCOPED_MUTEX;
			return _list.size(); 
		}

		/** @brief Gets the current size (in elements) of the cache.
		 *  @return size in elements
		 */
		inline const unsigned long max_size( void ) const { return _max_size; }

		/// Clears all storage and indices.
		void clear( void ) {
			SCOPED_MUTEX;
			_list.clear();
			_index.clear();
		};

		/** @brief Checks for the existance of a key in the cache.
		 *  @param key to check for
		 *  @return bool indicating whether or not the key was found.
		 */
#ifdef _REENTRANT
		inline bool exists( const Key &key ) {
			SCOPED_MUTEX;
#else
		inline bool exists( const Key &key ) const {
#endif
			return _index.find( key ) != _index.end();
		}

		/** @brief Removes a key-data pair from the cache.
		 *  @param key to be removed
		 */
#ifdef _REENTRANT
		inline void remove( const Key &key ) {
			SCOPED_MUTEX;
#else
		inline void remove( const Key &key ) const {
#endif
			Map_Iter miter = _index.find( key );
			if( miter == _index.end() ) return;
			_remove( miter );
		}

		/** @brief Touches a key in the Cache and makes it the most recently used.
		 *  @param key to be touched
		 */
		inline void touch( const Key &key ) {
			SCOPED_MUTEX;
			_touch( key );
		}

		/** @brief Fetches a pointer to cache data.
		 *  @param key to fetch data for
		 *  @param touch whether or not to touch the data
		 *  @return pointer to data or NULL on error
		 */
		inline Data *fetch_ptr( const Key &key, bool touch = true ) {
			SCOPED_MUTEX;
			Map_Iter miter = _index.find( key );
			if( miter == _index.end() ) return NULL;
			this->_touch( key );
			return &(miter->second->second);
		}

		/** @brief Fetches a copy of cached data.
		 *  @param key to fetch data for
		 *  @param touch_data whether or not to touch the data
		 *  @return copy of the data or an empty Data object if not found
		 */
		inline Data fetch( const Key &key, bool touch_data = true ) {
			SCOPED_MUTEX;
			Map_Iter miter = _index.find( key );
			if( miter == _index.end() )
				return Data();
			Data tmp = miter->second->second;
			if( touch_data )
				_touch( key );
			return tmp;
		}

		/** @brief Inserts a key-data pair into the cache and removes entries if neccessary.
		 *  @param key object key for insertion
		 *  @param data object data for insertion
		 *  @note This function checks key existance and touches the key if it already exists.
		 */
		inline void insert( const Key &key, const Data &data ) {
			SCOPED_MUTEX;
			// Touch the key, if it exists, then replace the content.
			Map_Iter miter = this->_touch( key );
			if( miter != _index.end() )
				this->_remove( miter );

			// Ok, do the actual insert at the head of the list
			_list.push_front( std::make_pair( key, data ) );
			List_Iter liter = _list.begin();

			// Store the index
			_index.insert( std::make_pair( key, liter ) );

			// Check to see if we need to remove an element due to exceeding max_size
			if( _list.size() > _max_size ) {
				// Remove the last element.
				liter = _list.end();
				--liter;
				this->_remove( liter->first );
			}
		}

		/** @brief Get a list of keys.
				@return list of the current keys.
		*/
		inline const Key_List get_all_keys( void ) {
			SCOPED_MUTEX;
			Key_List ret;
			for( List_cIter liter = _list.begin(); liter != _list.end(); liter++ )
				ret.push_back( liter->first );
			return ret;
		}

	private:
		/** @brief Internal touch function.
		 *  @param key to be touched
		 *  @return a Map_Iter pointing to the key that was touched.
		 */
		inline Map_Iter _touch( const Key &key ) {
			Map_Iter miter = _index.find( key );
			if( miter == _index.end() ) return miter;
			// Move the found node to the head of the list.
			_list.splice( _list.begin(), _list, miter->second );
			return miter;
		}

		/** @brief Interal remove function
		 *  @param miter Map_Iter that points to the key to remove
		 *  @warning miter is now longer usable after being passed to this function.
		 */
		inline void _remove( const Map_Iter &miter ) {
			_list.erase( miter->second );
			_index.erase( miter );
		}

		/** @brief Interal remove function
		 *  @param key to remove
		 */
		inline void _remove( const Key &key ) {
			Map_Iter miter = _index.find( key );
			_list.erase( miter->second );
			_index.erase( miter );
		}
};
