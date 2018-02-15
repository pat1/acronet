
#ifndef MODULE_PANEL_H_
#define MODULE_PANEL_H_


typedef struct PANEL_DATA
{
	uint8_t			status;
} PANEL_DATA;



RET_ERROR_CODE panel_init(void);


void panel_get_data(PANEL_DATA * const ps);
void panel_set_data(PANEL_DATA * const ps);

void panel_reset_data(void);

RET_ERROR_CODE panel_Data2String(const PANEL_DATA * const st,char * const sz, int16_t * len_sz);




#endif /* MODULE_PANEL_H_ */
