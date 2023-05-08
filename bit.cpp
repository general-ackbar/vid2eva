       #include <stdio.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

int main(int argc, char *argv[])
{

    unsigned char inbyte = 0x1A;
    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(inbyte));
    


    unsigned char firstPart = (inbyte & 0b11000000) >> 6;
    unsigned char secondPart = (inbyte & 0b00110000) >> 4;
    unsigned char thirdPart = (inbyte & 0b00001100) >> 2;
    unsigned char fourthPart = (inbyte & 0b00000011);

    printf(BYTE_TO_BINARY_PATTERN", value: %i\n", BYTE_TO_BINARY( firstPart ), (int)firstPart);
    printf(BYTE_TO_BINARY_PATTERN", value: %i\n", BYTE_TO_BINARY( secondPart), (int)secondPart);
    printf(BYTE_TO_BINARY_PATTERN", value: %i\n", BYTE_TO_BINARY( thirdPart), (int)thirdPart);
    printf(BYTE_TO_BINARY_PATTERN", value: %i\n", BYTE_TO_BINARY( fourthPart), (int)fourthPart);

    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY( 0x03 << 6 ) );
    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY( 0x03 << 4 ) );
    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY( (0x00 | 0x03) << 2 ) );
    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY( (0x00 | 0x03) ) );

}