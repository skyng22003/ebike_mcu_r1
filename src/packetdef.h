
#include <stdint.h>




#pragma once

typedef struct {
	uint8_t pk[216];
}RSA_1024_pk_t;

typedef struct  {
	uint8_t pk[128];
}RSA_1024_crypt_dat_t;

typedef struct {
	uint8_t key[16];
}AES128_t;

typedef struct {
	uint8_t key[32];
}AES256_t;


uint8_t keys[32];


typedef struct RSA_2048_pk_t {
	uint8_t pk[371];
};

typedef struct ESP32_dev_id_t {
	char id[23];

};

typedef struct GetVal_t {
	uint8_t Type;
};



class ESP_RXData {

public:
	typedef enum Type
	{
    RSA_1024_PK =1,
	RSA_2048_PK,
	AES_128_CRYPT_DAT,
	AES_256_CRYPT_DAT,
	GET_VALUES
	};

	RSA_1024_pk_t RSA_1024_pk;
	RSA_2048_pk_t RSA_2048_pk;
	GetVal_t GetVal;
};



class ESP_TXData {

public:
	typedef enum Type
	{
		ESP32_DEV_ID = 1,
		RSA_1024_CRYPT_DAT,
		RSA_2048_CRYPT_DAT,
		AES_128_CRYPT_KEY,
		AES_256_CRYPT_KEY,
		GET_VALUES
	};
	ESP32_dev_id_t ESP32_dev_id;
	RSA_1024_crypt_dat_t RSA_1024_pk;
	RSA_2048_pk_t RSA_2048_pk;
	AES128_t AES128;
	AES256_t AES256;
	GetVal_t GetVal;
};



typedef enum 
{
	PROCESS_HEAD_PACKET = 0,
	PROCESS_PAYLOAD_TYPE,
	PROCESS_GET_PACKET,
	PROCESS_RSA_1024_PACKET
}RX_State;



