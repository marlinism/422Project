/*
 * This is a C implementation of malloc( ) and free( ), based on the buddy
 * memory allocation algorithm. 
 */
#include <stdio.h> // printf

/*
 * The following global variables are used to simulate memory allocation
 * Cortex-M's SRAM space.
 */
// Heap
char array[0x8000];            // simulate SRAM: 0x2000.0000 - 0x2000.7FFF
int heap_top   = 0x20001000;   // the top of heap space
int heap_bot   = 0x20004FE0;   // the address of the last 32B in heap
int max_size   = 0x00004000;   // maximum allocation: 16KB = 2^14
int min_size   = 0x00000020;   // minimum allocation: 32B = 2^5

// Memory Control Block: 2^10B = 1KB space
int mcb_top    = 0x20006800;   // the top of MCB
int mcb_bot    = 0x20006BFE;   // the address of the last MCB entry
int mcb_ent_sz = 0x00000002;   // 2B per MCB entry
int mcb_total  = 512;          // # MCB entries: 2^9 = 512 entries

/*
 * Convert a Cortex SRAM address to the corresponding array index.
 * @param  sram_addr address of Cortex-M's SRAM space starting at 0x20000000.
 * @return array index.
 */
int m2a( int sram_addr ) {
	// TODO: part 1 - add comment to each of the following line of code
	int index = sram_addr - 0x20000000;
	return index;
}

/*
 * Reverse an array index back to the corresponding Cortex SRAM address.
 * @param  array index.
 * @return the corresponding Cortex-M's SRAM address in an integer.
 */ 
int a2m( int array_index ) {
	//  TODO: part 1 - add comment to each of the following line of code
	return array_index + 0x20000000;
}

/*
 * In case if you want to print out, all array elements that correspond
 * to MCB: 0x2006800 - 0x20006C00.
 */
void printArray( ) {
	printf( "memroy ............................\n" );
	// T TODO: part 1 - add comment to each of the following line of code
	for ( int i = 0; i < 0x8000; i+=4 )
		if ( a2m( i ) >= 0x20006800 ) 
			printf( "%x = %x(%d)\n", a2m( i ), *(int *)&array[i], *(int *)&array[i] ); 
}

/*
 * _ralloc is _kalloc's helper function that is recursively called to
 * allocate a requested space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  size  the size of a requested memory space
 * @param  left_mcb_addr  the address of the left boundary of MCB entries to examine
 * @param  right_mcb_addr the address of the right boundary of MCB entries to examine
 * @return the address of Cortex-M's SRAM space. While the computation is
 *         made in integers, cast it to (void *). The gcc compiler gives
 *         a warning sign:
                cast to 'void *' from smaller integer type 'int'
 *         Simply ignore it.
 */
void *_ralloc( int size, int left_mcb_addr, int right_mcb_addr ) {
	// initial parameter computation
	//  TODO: part 1 - add comment to each of the following line of code
	int entire_mcb_addr_space = right_mcb_addr - left_mcb_addr  + mcb_ent_sz;
	int half_mcb_addr_space = entire_mcb_addr_space / 2;
	int midpoint_mcb_addr = left_mcb_addr + half_mcb_addr_space;
	int heap_addr = 0;
	int act_entire_heap_size = entire_mcb_addr_space * 16;
	int act_half_heap_size = half_mcb_addr_space * 16;

	// base case
	//  TODO: part 1 - add comment to each of the following line of code
	if ( size <= act_half_heap_size ) {
		void* heap_addr = _ralloc( size, left_mcb_addr, midpoint_mcb_addr - mcb_ent_sz );
		if ( heap_addr == 0 ) {
			return _ralloc( size, midpoint_mcb_addr, right_mcb_addr );
		}
		if ( ( array[ m2a( midpoint_mcb_addr ) ] & 0x01 ) == 0 )
			*(short *)&array[ m2a( midpoint_mcb_addr ) ] = act_half_heap_size;
		return heap_addr;
	}
	else {
		if ( ( array[ m2a( left_mcb_addr ) ] & 0x01 ) != 0 ) {
			return 0;
		}
		else {
			if ( *(short *)&array[ m2a( left_mcb_addr ) ] < act_entire_heap_size )
				return 0;
			*(short *)&array[ m2a( left_mcb_addr ) ] = act_entire_heap_size | 0x01;
			return (void *)( heap_top + ( left_mcb_addr - mcb_top ) * 16 );
		}
	}
	return 0;
}

/*
 * _rfree is _kfree's helper function that is recursively called to
 * deallocate a space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  mcb_addr that corresponds to a SRAM space to deallocate
 * @return the same as the mcb_addr argument in success, otherwise 0.
 */
int _rfree( int mcb_addr ) {
	//  TODO: part 1 - add comment to each of the following line of code
	short mcb_contents = *(short *)&array[ m2a( mcb_addr ) ];
	int mcb_index = mcb_addr - mcb_top;
	int mcb_disp = ( mcb_contents /= 16 );
	int my_size = ( mcb_contents *= 16 );

	// mcb_addr's used bit was cleared
	*(short *)&array[ m2a( mcb_addr ) ] = mcb_contents;
	
	//  TODO: part 1 - add comment to each of the following line of code
	if ( ( mcb_index / mcb_disp ) % 2 == 0 ) {  
		if ( mcb_addr + mcb_disp >= mcb_bot )
			return 0; // my buddy is beyond mcb_bot!
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr + mcb_disp ) ];   
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {
				mcb_buddy = ( mcb_buddy / 32 ) * 32;
				if ( mcb_buddy == my_size ) {
					*(short *)&array[ m2a( mcb_addr + mcb_disp ) ] = 0;
					my_size *= 2;
					*(short *)&array[ m2a( mcb_addr ) ] = my_size;
					return _rfree( mcb_addr );
				}
			}
		}
	}
	else {  
		if ( mcb_addr - mcb_disp < mcb_top )
			return 0; // my buddy is below mcb_top!
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr - mcb_disp ) ];
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {
				mcb_buddy = ( mcb_buddy / 32 ) * 32;
				if ( mcb_buddy == my_size ) {
					*(short *)&array[ m2a( mcb_addr ) ] = 0;
					my_size *= 2;
					*(short *)&array[ m2a( mcb_addr - mcb_disp ) ] = my_size;
					return _rfree( mcb_addr - mcb_disp );
				}
			}
		}
	}
  
  return mcb_addr;
}

/*
 * Initializes MCB entries. In step 2's assembly coding, this routine must
 * be called from Reset_Handler in startup_TM4C129.s before you invoke
 * driver.c's main( ).
 */
void _kinit( ) {
	//  TODO: part 1 - add comment to each of the following line of code
	for ( int i = 0x20001000; i < 0x20005000; i++ )
		array[ m2a( i ) ] = 0;

		*(short *)&array[ m2a( mcb_top ) ] = max_size;

	for ( int i = 0x20006804; i < 0x20006C00; i += 2 ) {
		array[ m2a( i ) ] = 0;
		array[ m2a( i + 1) ] = 0;
	}
}

/*
 * Step 2 should call _kalloc from SVC_Handler.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_kalloc( int size ) {
  return _ralloc( size, mcb_top, mcb_bot );
}

/*
 * Step 2 should call _kfree from SVC_Handler.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_kfree( void *ptr ) {
	//  TODO: part 1 - add comment to each of the following line of code
	int addr = (int )ptr;

	if ( addr < heap_top || addr > heap_bot )
		return NULL;
	int mcb_addr =  mcb_top + ( addr - heap_top ) / 16;

	if ( _rfree( mcb_addr ) == 0 )
		return NULL;
	else
		return ptr;
}

/*
 * _malloc should be implemented in stdlib.s in step 2.
 * _kalloc must be invoked through SVC in step 2.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_malloc( int size ) {
	static int init = 0;
	if ( init == 0 ) {
		init = 1;
		_kinit( );
	}
	return _kalloc( size );
}

/*
 * _free should be implemented in stdlib.s in step 2.
 * _kfree must be invoked through SVC in step 2.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_free( void *ptr ) {
	return _kfree( ptr );
}
