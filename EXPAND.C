#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* expand(fbuf, inbuf, flen); */
int	expand(uint8_t *q, uint8_t *p, int l)
{
	uint8_t	*q0;
	
	q0 = q;
	do {
		uint8_t	s1=0, s2=0, s3=0;
		uint8_t	c;

		c = *p++;

/* printf("count = %5d     flag = %02X\n", l, c); 
*/
		switch(c & 0xC0)
		{
			case 0x40: s1 = (c & 63);
				   break;
			case 0x80: s2 = (c & 63);
				   break;
			case 0xC0: s1 = ((c >> 3) & 7);
				   s3 = (c & 7);
				   break;
			default:   s1 = ((c >> 3) & 7);
				   s2 = (c & 7);
		}
		
		if(s1)
		{	q += s1;
		}
		
		if(s2)
		{
			do {
				*q++ = *p++;
				l--;
			} while(--s2);
		}
		
		if(s3)
		{	int	c;
			--q;
			c = *q;
			q++;
			do {
				*q++ = c;
			} while(--s3);
		}
	} while(--l);
	
	return(q - q0);
}
