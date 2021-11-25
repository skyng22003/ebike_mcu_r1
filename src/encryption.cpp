

#include <Arduino.h>
#include <mbedtls/config.h>
#include <mbedtls/platform.h>
#include <mbedtls/error.h>
#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include "SPIFFS.h"
#include "packetdef.h"




#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

/* mbed TLS feature support */
#define MBEDTLS_PKCS1_V15

/* mbed TLS modules */
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_MD_C
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_RSA_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_USE_C

#define ESP32_ID_FORMAT "MCUDEVICE-%04X%08X"
#define ESP32_ID_LENGTH 23





ESP_RXData ESP_RX;
ESP_TXData ESP_TX;


unsigned char key_begin[] = "-----BEGIN PUBLIC KEY-----\n";

unsigned char key_end[] = "-----END PUBLIC KEY-----\n";


unsigned char pubkey[216];

static const unsigned char* SIGNING_PUB_KEY_1 = (unsigned char*)
"-----BEGIN PUBLIC KEY-----\n"
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC+ZNgsEoRdQheJirMokoZVxlzhXxpHBjN7pVY3\n"
"Z5AzbB0cx4lglpNxiDbr+5S8nJfY2iZEFlItV4EJezwYrAtR2jShi87GS6wkB5Yi+9oT7vwMQTrL\n"
"6zz4KdlrF8Yum+l6wtqXJl6sklzn3wspeOfEFjZwQvvCBebXTIeiB92SdQIDAQAB\n"
"-----END PUBLIC KEY-----\n";


static const unsigned char* SIGNING_PUB_KEY = (unsigned char*)
"-----BEGIN PUBLIC KEY-----\n"


"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4QpZrhrhMneE2eTcBuv2RRxAjRXs7q7e\n"
"1QIJa2n7gtJEMRprNAqwPzJ4hyjCrnPV3kSsbECCfeAPUMqW9sNIdHImhKrybdgA70uoUgkib2aU\n"
"EinyacFmfrBpC8UrjZq/vsl10MQUvURjo0eZzovbQwnvDj3lBQpmcQ/uMQcMgEWyd7vj4hH1fI90\n"
"8edG8LLpjCCHYp1N5L2N5uFANP2sCopSCTJhh5DF4ffOCDguCa15KblAMmlj+ary2G82Bj4+NTP6\n"
"VLGNCDMCOKNQ+y8C4sXTi2OxWJsrwWQsJ/8TBRnaaFemhXVRsTPV3e1TRmCKdb4fMzb0XD+DpWSE\n"
"jQJ96QIDAQAB\n"






"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAr+q/H5DxV1etGxTNWtns\n"
"xmVkiAKVPEcejoNT2xObwsQ/WTq7RtOAUOkIY3Yd63S3RFcdmlIy4NFEg+ccrqpu\n"
"4z/Zi0AHELDk28WxMxLj1tr7oz/f0DbyjpzyRW/6Cm9RMfGtASiJFmARlQiVaZ+f\n"
"/QemObAJfnJLEbqOJeIh6byJ/dUCWTIVqaK/2n+1hBsfcFq8OAAXG3fMOVipOSNc\n"
"raL6FxPrZ2iVS6OvbTV2PmiF8PU4KSIPVNj1Nfe3Gc2W7q/NRkMFWfhOg5zO7YRd\n"
"eR4t8ZlOdURpKyIwRKiBDXx9jUedagswu7RqWC2/9XB1exP9N8A4PMCTLnweljEc\n"
"ewIDAQAB\n"
"-----END PUBLIC KEY-----\n";




static uint8_t rx_data[218];
mbedtls_pk_context pk;






bool HostRx() {
    static uint8_t rx_buf[254];
    uint8_t rx_byte;
    static uint8_t rx_state = PROCESS_HEAD_PACKET;
    static uint16_t rx_cnt = 0;

    rx_byte = Serial.read();
    switch (rx_state)
    {
    case PROCESS_HEAD_PACKET:
        if (rx_byte == 0x60) {
            //delay(10);
            //Serial.println("HeadByteReceived");
            rx_state = PROCESS_PAYLOAD_TYPE;
            break;
        }

        else {
            rx_state = PROCESS_HEAD_PACKET;
            return false;
            break;
        }

    case PROCESS_PAYLOAD_TYPE:
        if (rx_byte == ESP_RX.GET_VALUES) {
            rx_state = PROCESS_GET_PACKET;
            //delay(10);
            //Serial.println("Got GET Head");
            break;
        }


        if (rx_byte == ESP_RX.RSA_1024_PK) {
            uint8_t length = Serial.read();
              if (length == sizeof(ESP_RX.RSA_1024_pk.pk)) {

                  rx_state = PROCESS_RSA_1024_PACKET;
                  //delay(10);
                  //Serial.println("Got RSA Head");
                  break;
              }

              else
              {
                  rx_state = PROCESS_HEAD_PACKET;
                  break;
              }
           
        }
        else {
            rx_state = PROCESS_HEAD_PACKET;
            break;
        }



    case PROCESS_GET_PACKET:
        if (rx_byte == ESP_TX.ESP32_DEV_ID) {
            uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
            uint16_t chip = (uint16_t)(chipid >> 32);
            snprintf(ESP_TX.ESP32_dev_id.id, sizeof(ESP_TX.ESP32_dev_id.id), "MCUDEVICE-%04X%08X", chip, (uint32_t)chipid);
            delay(10);
            Serial.write(0x60);
            Serial.write(ESP_TX.ESP32_DEV_ID);
            Serial.write(sizeof(ESP_TX.ESP32_dev_id.id));
            Serial.write((uint8_t*)ESP_TX.ESP32_dev_id.id, sizeof(ESP_TX.ESP32_dev_id.id));
           // delay(10);
           // Serial.print("Sent Device ID");
            rx_state = PROCESS_HEAD_PACKET;
            return true;
            break;
        }




        if (rx_byte == ESP_TX.AES_128_CRYPT_KEY) {


            mbedtls_ctr_drbg_context ctr_drbg;
            mbedtls_entropy_context entropy;
            unsigned char key[16];

            char* pers = "aes generate key";
            int ret;

            mbedtls_entropy_init(&entropy);
            mbedtls_ctr_drbg_init(&ctr_drbg);


            if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                (unsigned char*)pers, strlen(pers))) != 0)
            {
                if ((ret = mbedtls_ctr_drbg_random(&ctr_drbg, key, 16)) != 0)
                {
                    mbedtls_pk_context pk;
                    mbedtls_pk_init(&pk);

                    int pkret;
                    unsigned char t_key[sizeof(key_begin) + sizeof(ESP_RX.RSA_1024_pk.pk) + sizeof(key_end)];
                    strcpy((char*)t_key, (char*)key_begin);
                    strcat((char*)t_key, (char*)ESP_RX.RSA_1024_pk.pk);
                    strcat((char*)t_key, (char*)key_end);
                    const unsigned char* m_key = (const unsigned char*)t_key;
                    pkret = mbedtls_pk_parse_public_key(&pk, m_key, strlen((char*)m_key) + 1);



                }
            }


            /*
            mbedtls_aes_context aes;
          
            mbedtls_aes_init(&aes);
            mbedtls_aes_setkey_enc(&aes, (const unsigned char*)key, strlen(key) * 8);
            mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)input, output);
            mbedtls_aes_free(&aes);
            */





        }





        else {
            rx_cnt = 0;
            rx_state = PROCESS_HEAD_PACKET;
            return false;
            break;

        }

    case PROCESS_RSA_1024_PACKET:
    //Serial.println("PROCESS_DATA_PACKET");

        if (rx_cnt <= sizeof(ESP_RX.RSA_1024_pk.pk)) {
            //delay(100);
           // Serial.println("Got RSA Key Head");
            rx_buf[rx_cnt] = rx_byte;
         //   ESP_RX.RSA_1024_pk.pk[rx_cnt] = rx_byte;
         //  Serial.print(rx_cnt);
          // Serial.print(":");
          // Serial.println((char)ESP_RX.RSA_1024_pk.pk[rx_cnt]);
           
          if (rx_cnt == sizeof(ESP_RX.RSA_1024_pk.pk)-1) {
              memcpy(ESP_RX.RSA_1024_pk.pk, rx_buf, sizeof(ESP_RX.RSA_1024_pk.pk));
                Serial.println("Got RSA Key");
                mbedtls_pk_context pk;
                mbedtls_pk_init(&pk);
                
                int pkret;
                unsigned char t_key[sizeof(key_begin) + sizeof(ESP_RX.RSA_1024_pk.pk) + sizeof(key_end)];
                strcpy((char*)t_key, (char*)key_begin);
                strcat((char*)t_key, (char*)ESP_RX.RSA_1024_pk.pk);
                strcat((char*)t_key, (char*)key_end);
                Serial.print((char*)t_key);
                Serial.println();
                const unsigned char* m_key = (const unsigned char*)t_key;


                pkret = mbedtls_pk_parse_public_key(&pk, m_key, strlen((char*)m_key) + 1);
                if (pkret == 0) {
                    Serial.println("Public key Parsed OK");
                    mbedtls_ctr_drbg_context ctr_drbg;
                    mbedtls_ctr_drbg_context ctr_drbg_crypt;
                    mbedtls_entropy_context entropy_crypt;
                    char* personalization = "my_app_specific_string";
                    mbedtls_entropy_init(&entropy_crypt);
                    mbedtls_ctr_drbg_init(&ctr_drbg_crypt);
                    mbedtls_ctr_drbg_seed(&ctr_drbg_crypt, mbedtls_entropy_func, &entropy_crypt, 0, 0);
                    unsigned char encrypted[MBEDTLS_MPI_MAX_SIZE];
                    size_t olen = 0;
                    int ret = 0;
                    size_t to_encrypt_len = sizeof(ESP_TX.ESP32_dev_id.id);
                    Serial.println("About to Encrypt");


                    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy_crypt,
                        (const unsigned char*)personalization,
                        strlen(personalization));

                    ret = mbedtls_pk_encrypt(&pk, (uint8_t*)ESP_TX.ESP32_dev_id.id, to_encrypt_len,
                        encrypted, &olen, sizeof(encrypted),
                        mbedtls_ctr_drbg_random, &ctr_drbg);
                    if (ret == 0)
                    {
                        
                        uint8_t length = olen;
                        Serial.println("Encyption OK");
                        //Serial.println(length);
                        Serial.write(0x60);
                        Serial.write(ESP_TX.RSA_1024_CRYPT_DAT);
                        Serial.write(length);
                        Serial.write(encrypted, olen);
                        mbedtls_entropy_free(&entropy_crypt);
                        mbedtls_ctr_drbg_free(&ctr_drbg_crypt);
                        mbedtls_pk_free(&pk);
                        rx_cnt = 0;
                        rx_state = PROCESS_HEAD_PACKET;
                        break;

                    }

                    if (ret != 0)
                    {
                        Serial.println("Encyption Failed");
                    }


                    return true;
                }

                if (pkret != 0) {
                    Serial.println("Public key parsing has failed");
                    return false;
                }

               







                rx_cnt = 0;
                rx_state = PROCESS_HEAD_PACKET;
                break;
                
          }
            ++rx_cnt;
            if (rx_cnt > 215) {
                rx_cnt = 0;
                rx_state = PROCESS_HEAD_PACKET;
                break;
            }
        }

        else {
            return false;
            rx_cnt = 0;
            rx_state = PROCESS_HEAD_PACKET;
            break;

        }






    }
}


static void parse_from_buffer_test() {
    int pkret;
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    // Initialize our public key used to verify the signature
    pkret = mbedtls_pk_parse_public_key(&pk, SIGNING_PUB_KEY_1, strlen((const char*)SIGNING_PUB_KEY_1) + 1);
    if (pkret == 0) {
        Serial.println("Public key Parsed OK");
    }
    if (pkret != 0) {
        char buff[128];
        //    getTlsError(pkret,buff,sizeof(buff));
        Serial.println("Public key parsing has failed");
    }
}




void get_ESP32_ID(char* id) {
    uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(id, ESP32_ID_LENGTH, ESP32_ID_FORMAT, chip, (uint32_t)chipid);
}



bool save_key(char* KeyFilePath, unsigned char* key) {
    File keyfile;
    keyfile = SPIFFS.open(KeyFilePath, FILE_WRITE);
    if (keyfile) {

        keyfile.print((char*)key);
        return true;
    }
    else {
        return false;
    }
}


/*
static void parse_from_buffer(const unsigned char* key) {
    int pkret;
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    // Initialize our public key used to verify the signature
    pkret = mbedtls_pk_parse_public_key(&pk, key, strlen((const char*)key) + 1);
    if (pkret == 0) {
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ctr_drbg_context ctr_drbg_crypt;
        mbedtls_entropy_context entropy_crypt;
        char* personalization = "my_app_specific_string";
        mbedtls_entropy_init(&entropy_crypt);
        mbedtls_ctr_drbg_init(&ctr_drbg_crypt);
        mbedtls_ctr_drbg_seed(&ctr_drbg_crypt, mbedtls_entropy_func, &entropy_crypt, 0, 0);

        Serial.println("Public key Parsed OK");
        unsigned char encrypted[MBEDTLS_MPI_MAX_SIZE];
        size_t olen = 0;
        int ret = 0;
        unsigned char to_encrypt[] = "Hello 1 2 3 4 5";
        size_t to_encrypt_len = sizeof(to_encrypt);
        Serial.println("About to Encrypt");


        ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy_crypt,
            (const unsigned char*)personalization,
            strlen(personalization));


        ret = mbedtls_pk_encrypt(&pk, to_encrypt, to_encrypt_len,
            encrypted, &olen, sizeof(encrypted),
            mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret == 0)
        {
            Serial.println("Encyption OK");
            //unsigned char encrypted_data[olen];
            //strcpy ((char*)encrypted_data, (char*)encrypted);
          // memcpy(encrypted_data, encrypted, olen);
            //const unsigned char* encrypted_data_s = (const unsigned char*) encrypted_data;
            //   Serial.println((char*)encrypted_data);
            for (int i = 0; i < olen; i++) {
                // Serial.printf("%02x", encrypted[i]);
            }
            //Serial.println();
            delay(100);
            Serial.write(0x60);
            Serial.write(0x20);
            Serial.write(encrypted, olen);
        }

        if (ret != 0)
        {
            Serial.println("Encyption Failed");
        }

    }
    if (pkret != 0) {
        char buff[128];
        //    getTlsError(pkret,buff,sizeof(buff));
        Serial.println("Public key parsing has failed");
    }
}




bool uart_rx_org() {
    static uint8_t rx_buf[218];
    uint8_t rx_byte;
    static uint8_t rx_state = 0;
    static uint16_t rx_cnt = 0;

    rx_byte = Serial.read();

    switch (rx_state)
    {
    case 0:
        if (rx_byte == 0x60) {
            Serial.println("HeadByteReceived");
            rx_state = 1;
            break;
        }

        else {
            rx_state = 0;
            return false;
            break;
        }

    case 1:
        if (rx_byte == 0x30) {
            char ssid[23];
            uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
            uint16_t chip = (uint16_t)(chipid >> 32);
            snprintf(ssid, 23, "MCUDEVICE-%04X%08X", chip, (uint32_t)chipid);
            delay(100);
            Serial.write(0x60);
            Serial.write(0x30);
            Serial.write((uint8_t*)ssid, 23);
            rx_state = 0;
            return true;
            break;
        }


        if (rx_byte == 0x20) {
            rx_state = 2;
            break;
        }
        else {
            rx_state = 0;
            break;
        }

    case 2:
        if (rx_cnt < 218) {
            rx_buf[rx_cnt] = rx_byte;
            //Serial.print(rx_cnt);
            //Serial.print(":");
            //Serial.println((char)rx_buf[rx_cnt]);
            //delay(1);
            ++rx_cnt;
        }
        if (rx_cnt == 218) {

            memcpy(rx_data, rx_buf, 218);
            Serial.println("KeyReceived");
            //String key = String((char*)rx_data);
            //const unsigned char* key_1 = (const unsigned char*)rx_data;
            //Serial.println(strlen((const char *)key_1));

            unsigned char t_key[sizeof(key_begin) + sizeof(rx_data) + sizeof(key_end)];
            strcpy((char*)t_key, (char*)key_begin);
            strcat((char*)t_key, (char*)rx_data);
            strcat((char*)t_key, (char*)key_end);
            Serial.print((char*)t_key);
            Serial.println();
            const unsigned char* m_key = (const unsigned char*)t_key;
            parse_from_buffer(m_key);
            rx_cnt = 0;
            rx_state = 0;
            return true;
            break;
        }

        if (rx_cnt > 217) {
            rx_cnt = 0;
            return false;
            break;
        }
        break;

    }


}



bool parse_key(uint8_t* array) {
    int pkret;
    unsigned char t_key[sizeof(key_begin) + sizeof(array) + sizeof(key_end)];
    strcpy((char*)t_key, (char*)key_begin);
    strcat((char*)t_key, (char*)array);
    strcat((char*)t_key, (char*)key_end);
    Serial.print((char*)t_key);
    Serial.println();
    const unsigned char* m_key = (const unsigned char*)t_key;


    pkret = mbedtls_pk_parse_public_key(&pk, m_key, strlen((char*)m_key) + 1);
    if (pkret == 0) {
        Serial.println("Public key Parsed OK");
        return true;
    }

    if (pkret != 0) {
        Serial.println("Public key parsing has failed");
        return false;
    }
}


void RSA_encrypt(unsigned char* to_encrypt)
{
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_context ctr_drbg_crypt;
    mbedtls_entropy_context entropy_crypt;
    char* personalization = "my_app_specific_string";
    mbedtls_entropy_init(&entropy_crypt);
    mbedtls_ctr_drbg_init(&ctr_drbg_crypt);
    mbedtls_ctr_drbg_seed(&ctr_drbg_crypt, mbedtls_entropy_func, &entropy_crypt, 0, 0);
    unsigned char encrypted[MBEDTLS_MPI_MAX_SIZE];
    size_t olen = 0;
    int ret = 0;
    size_t to_encrypt_len = sizeof(to_encrypt);
    Serial.println("About to Encrypt");


    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy_crypt,
        (const unsigned char*)personalization,
        strlen(personalization));

    ret = mbedtls_pk_encrypt(&pk, to_encrypt, to_encrypt_len,
        encrypted, &olen, sizeof(encrypted),
        mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret == 0)
    {
        Serial.println("Encyption OK");
        //  unsigned char encrypted_data[olen];
          //strcpy ((char*)encrypted_data, (char*)encrypted);
        // memcpy(encrypted_data, encrypted, olen);
          //const unsigned char* encrypted_data_s = (const unsigned char*) encrypted_data;
          //   Serial.println((char*)encrypted_data);
        for (int i = 0; i < olen; i++) {
            // Serial.printf("%02x", encrypted[i]);
        }
        //Serial.println();
        delay(100);
        Serial.write(0x60);
        Serial.write(ESP_TX.RSA_1024_CRYPT_DAT);
        Serial.write(olen);
        Serial.write(encrypted, olen);
    }

    if (ret != 0)
    {
        Serial.println("Encyption Failed");
    }
}
*/