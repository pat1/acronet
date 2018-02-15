/*
 * ACRONET Project
 * http://www.acronet.cc
 *
 * Copyright ( C ) 2014 Acrotec srl
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 *
 * 
 * 
 */


#ifndef SIM900_H_
#define SIM900_H_

#include "Acronet/drivers/UART_INT/buffer_usart.h"


void sim900_power_on(void);
//RET_ERROR_CODE sim900_set_DNS(void);
USART_data_t * SIM900_get_PUSART( void );
void sim900_power_off(void);


uint8_t sim900_send_sms(char * const p_sms_buf,const uint8_t isBufPGM, char * const p_sms_recipient,const uint8_t isRecipientPGM);

uint8_t sim900_send_sms_to_phone_book(const char * sms_book[],const uint8_t isBOOKPGM,char * const msg,const uint8_t isMSGPGM);

RET_ERROR_CODE sim900_init(void);

RET_ERROR_CODE sim900_get_APN_by_operator( char * const szAPN , uint16_t szAPNLen );

RET_ERROR_CODE sim900_bearer_simple_open(void);
RET_ERROR_CODE sim900_bearer_simple_close(void);
RET_ERROR_CODE sim900_bearer_simple_release(void);


RET_ERROR_CODE sim900_bearer_open(void );
RET_ERROR_CODE sim900_GPRS_check_line(void);

RET_ERROR_CODE sim900_bearer_close(void);

RET_ERROR_CODE sim900_http_read(char * const return_value, uint16_t * const rv_len, bool isBinary);

RET_ERROR_CODE sim900_http_get_prepare( const char * const get_URL, const uint8_t isURLPGM, uint16_t * const answerSize );
RET_ERROR_CODE sim900_http_get( const char * const get_URL, const uint8_t isURLPGM, char * const return_value, uint16_t * const rv_len, bool isBinary );
RET_ERROR_CODE sim900_http_post( const char * const post_URL, const uint8_t isURLPGM, const char * const post_data, const uint16_t len_post_data,const uint8_t isPostPGM );

RET_ERROR_CODE sim900_http_close(void);

uint8_t sim900_ftp_open(
	const char * const ftp_server,
	const uint8_t isSZServerPGM,
	const char * const ftp_username,
	const uint8_t isSZUserNamePGM,
	const char * const ftp_pwd,
	const uint8_t isSZPWDPGM
);


uint8_t sim900_ftp_put(
	const char * const file_name,
	const uint8_t isSZFNamePGM,
	const char * const file_path,
	const uint8_t isSZFilePathPGM,
	const char * const file_data,
	const uint8_t isFileDataPGM
);


RET_ERROR_CODE sim900_tcp_shutdown( void );
RET_ERROR_CODE sim900_tcp_close( const uint8_t cid );
RET_ERROR_CODE sim900_tcp_init( const uint8_t cid, const char * const service_APN, const uint8_t isSZPGM );
RET_ERROR_CODE sim900_tcp_connect( const uint8_t cid, const char * const service, const uint8_t isSZServicePGM, const char * port, const uint8_t isSZPortPGM);
RET_ERROR_CODE sim900_tcp_send(const uint8_t cid, const char * const pBuf, const uint8_t isBufPGM, const uint16_t dimBufSend, char * const pAnswer, uint16_t * const pdimBufAnsw);
RET_ERROR_CODE sim900_tcp_read( char * const pAnswer, uint16_t * const pdimBufAnsw);
RET_ERROR_CODE sim900_wait_closing_tcp_peer( void );
RET_ERROR_CODE sim900_tcp_get_buffer_len( const uint8_t cid, uint16_t * const pLen );

#endif /* SIM900_H_ */