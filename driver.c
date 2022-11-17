/*
 * This is a complete C program that can be compiled with gcc and executable 
 * on Linux and MacOS.
 * You cannot compile it in Keil uVision.
 * You do not modify this file.
 * You only use this file to understand how the required functions work in general. 
 * You can compile this file using: gcc/g++ driver.c -o ./a.out
 */


#include <string.h> // bzero, strncpy


#include <stdlib.h>  // malloc, free
#include <stdio.h>   // printf


int main( ) {
  char stringA[40] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabc\0";
  char stringB[40];
  
  bzero( stringB, 40 );
  strncpy( stringB, stringA, 40 );
  bzero( stringA, 40 );
  printf( "%s\n", stringA );
  printf( "%s\n", stringB );
  
  void* mem1 = malloc( 1024 );
  void* mem2 = malloc( 1024 );
  void* mem3 = malloc( 8192 );
  void* mem4 = malloc( 4096 );
  void* mem5 = malloc( 512 );
  void* mem6 = malloc( 1024 );
  void* mem7 = malloc( 512 );
  
  free( mem6 );
  free( mem5 );
  free( mem1 );
  free( mem7 );
  free( mem2 );
  
  void* mem8 = malloc( 4096 );
  
  free( mem4 );
  free( mem3 );
  free( mem8 );

  return 0;
}
