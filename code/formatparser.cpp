#include "formatparser.h"

#define	BUF		512
FFPasrer *	FFPasrer::instance = NULL;

struct FORMATDATA
{
	int datalengh;
	int formatid;
	unsigned char data[16];
};

FORMATDATA fformat[] = 
{
	{ 3,	FF_JPG, {0xFF, 0xD8, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 8,	FF_PNG, {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 2,	FF_BMP, {0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 3,	FF_PCX, {0x0A, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 3,	FF_PCX, {0x0A, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 3,	FF_PCX, {0x0A, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 12,	FF_TGA, {0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 12,	FF_TGA, {0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 6,	FF_GIF, {0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 6,	FF_GIF, {0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ -1,	FF_UNKNOWN, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} }
};

_FFINFO FFPasrer::parseFile(const wchar *filename)
{
	_FFINFO info;
	char inbuf[BUF];

	FILE *fp = NULL;
	errno_t err = _wfopen_s(&fp, filename, L"rb");
	if(fp == NULL) 
	{
		info.fileid = FF_ERROR;
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		int len  = _ftelli64(fp);
		fseek(fp, 0, SEEK_SET);

		if( len >= BUF )
		{
			int width, height;
			fread(inbuf, 1, BUF, fp);
			int id = analizeformat((unsigned char *)inbuf, width, height);
			if( id != FF_UNKNOWN )
			{
				info.width = width;
				info.height = height;
			}
			fclose(fp);
		}
		else
		{
			fclose(fp);
			info.fileid = FF_ERROR;
		}
	}

	return info;	
}

int FFPasrer::analizeformat(unsigned char *buf, int &width, int &height)
{
	int i = 0;
	while(1)
	{
		if( fformat[i].formatid == FF_UNKNOWN ) break;

		int ret = memcmp(buf, fformat[i].data, fformat[i].datalengh);
		if( ret == 0 )
		{
			switch(fformat[i].formatid)
			{
				case FF_GIF :
					{
						// 0x06 : 2 byte --> width
						// 0x08 : 2 byte --> height
						width = (buf[0x07] << 8) + buf[0x06];
						height = (buf[0x09] << 8) + buf[0x08];
					}
					break;

				case FF_TGA :
					{
						// 0x0c : 2 byte --> width
						// 0x0e : 2 byte --> height
						width = (buf[0x0D] << 8) + buf[0x0C];
						height = (buf[0x0F] << 8) + buf[0x0E];
					}
					break;

				case FF_BMP :
					{
						// 0x10 : 4 byte --> width
						// 0x14 : 4 byte --> height
						width = (buf[0x11] << 24) + (buf[0x10] << 16) + (buf[0x13] << 8) + (buf[0x12]);
						height = (buf[0x15] << 24) + (buf[0x14] << 16) + (buf[0x17] << 8) + (buf[0x16]);
					}
					break;

				case FF_PCX :
					{
#if(0)
						typedef struct _PcxHeader
						{
						  BYTE	Identifier;        /* PCX Id Number (Always 0x0A) */
						  BYTE	Version;           /* Version Number */
						  BYTE	Encoding;          /* Encoding Format */
						  BYTE	BitsPerPixel;      /* Bits per Pixel */
						  WORD	XStart;            /* Left of image */
						  WORD	YStart;            /* Top of Image */
						  WORD	XEnd;              /* Right of Image
						  WORD	YEnd;              /* Bottom of image */
						  WORD	HorzRes;           /* Horizontal Resolution */
						  WORD	VertRes;           /* Vertical Resolution */
						  BYTE	Palette[48];       /* 16-Color EGA Palette */
						  BYTE	Reserved1;         /* Reserved (Always 0) */
						  BYTE	NumBitPlanes;      /* Number of Bit Planes */
						  WORD	BytesPerLine;      /* Bytes per Scan-line */
						  WORD	PaletteType;       /* Palette Type */
						  WORD	HorzScreenSize;    /* Horizontal Screen Size */
						  WORD	VertScreenSize;    /* Vertical Screen Size */
						  BYTE	Reserved2[54];     /* Reserved (Always 0) */
						} 

						ImageWidth  = XEnd - XStart + 1; /* Width of image in pixels */
						ImageHeight = YEnd - YStart + 1; /* Length of image in scan lines */
#endif
						int XStart = (buf[0x05] << 8) + buf[0x04];
						int YStart = (buf[0x07] << 8) + buf[0x06];
						int XEnd = (buf[0x09] << 8) + buf[0x08];
						int YEnd = (buf[0x0B] << 8) + buf[0x0A];
						// 0x0F : 4 byte --> width
						// 0x13 : 4 byte --> height
						width = XEnd - XStart + 1;
						height = YEnd - YStart + 1;
					}
					break;

				case FF_JPG :
					{
						int position = -1;
						for (int i = 0; i < BUF-1; i++)
						{
							if (buf[i] == 0xFF && buf[i + 1] == 0xC0)
							{
								position = i;
								break;
							}
						}

						if (position > 0)
						{
							position = position + 5;
							height = buf[position] << 8 | buf[position + 1];
							width = buf[position + 2] << 8 | buf[position + 3];
						}
						else
						{
							width = 0;
							height = 0;
						}

// 						// 0xA5 : 2 byte --> width
// 						// 0xA3 : 2 byte --> height
// 						width = (buf[0xA5] << 8) + buf[0xA6];
// 						height = (buf[0xA3] << 8) + buf[0xA4];
					}
					break;

				case FF_PNG :
					{
						// byte order 바뀜
						// 0x10 : 4 byte --> width
						// 0x14 : 4 byte --> height
// 						width = (buf[0x10] << 24) + (buf[0x11] << 16) + (buf[0x12] << 8) + (buf[0x13]);
// 						height = (buf[0x14] << 24) + (buf[0x15] << 16) + (buf[0x16] << 8) + (buf[0x17]);
						width = (buf[16] << 24) + (buf[17] << 16) + (buf[18] << 8) + (buf[19]);
						height = (buf[20] << 24) + (buf[21] << 16) + (buf[22] << 8) + (buf[23]);
					}
					break;
			}

			return fformat[i].formatid;
		}
		i++;
	}

	return FF_UNKNOWN;
}

