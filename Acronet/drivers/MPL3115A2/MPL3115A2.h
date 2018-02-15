/*
 * ACRONET Project
 * http://www.acronet.cc
 *
 * Copyright ( C ) 2014 Acrotec srl
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 */


#ifndef MPL3115A2_H_
#define MPL3115A2_H_


RET_ERROR_CODE MPL3115A2_Write( const uint8_t cmd, const uint8_t data );
RET_ERROR_CODE MPL3115A2_Read( const uint8_t cmd, uint8_t * const data, const uint8_t len );



#endif /* MPL3115A2_H_ */