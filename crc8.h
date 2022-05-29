/*
	== crc8.h ==
	Atmega328P CRC8 計算 library
*/

#ifndef CRC8_H
#define CRC8_H

#ifdef __AVR_ARCH__
#include <avr/pgmspace.h>
#endif

// CRC計算の初期値。基本的に変更しなくてよい
#define CRC8INITVALUE 0x00

// 記号定数(多項式の名称)
#define CRC8NORM
//#define CRC8ATM
//#define CRC8DALLAS

// 多項式の値（最上位ビット省略）
#if defined(CRC8NORM)
#define CRC8POLY (0xD5) //CRC-8
#endif

#if defined(CRC8ATM)
#define CRC8POLY (0x07) //CRC-8-ATM (ATM Header Error Correction)
#endif

#if defined(CRC8DALLAS)
#define CRC8POLY (0x31) //CRC-8-Dallas/Maxim
#endif

// CRC8テーブルから計算。テーブルが用意されている場合のみ使用できます
unsigned char GetCRC8_TABLE(unsigned char *, unsigned char);

// CRC8の計算。任意のPOLYで使用できます
unsigned char GetCRC8(unsigned char *, unsigned char);

#endif
