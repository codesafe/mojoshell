#ifdef WIN32
#include <winsock2.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "FastXml.h"
#include "../stringutil.h"
#include "../utility.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif


/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** The MIT license:
**
** Permission is hereby granted, MEMALLOC_FREE of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

namespace FAST_XML
{

#define MAX_STACK 2048

class MyFastXml : public FastXml
{
public:
  enum CharType
  {
    CT_DATA,
    CT_EOF,
    CT_SOFT,
    CT_END_OF_ELEMENT, // either a forward slash or a greater than symbol
    CT_END_OF_LINE,
  };

  MyFastXml(void)
  {
    mInputData = 0;
    memset(mTypes,CT_DATA,256);
    mTypes[0] = CT_EOF;
    mTypes[32] = CT_SOFT;
    mTypes[9] = CT_SOFT;
    mTypes['/'] = CT_END_OF_ELEMENT;
    mTypes['>'] = CT_END_OF_ELEMENT;
    mTypes['?'] = CT_END_OF_ELEMENT;
    mTypes[10] = CT_END_OF_LINE;
    mTypes[13] = CT_END_OF_LINE;
    mError = 0;
    mStackIndex = 0;
  }

  virtual ~MyFastXml(void)
  {
    release();
  }

  bool checkTypes(wchar t, int i)
  {
	  if( t >= 256 ) return false;
	  return mTypes[t] == i;
  }

  void release(void)
  {
    mError = 0;
    if ( mInputData )
    {
    	::free(mInputData);
    	mInputData = 0;
    }
  }

  inline wchar *nextSoft(wchar *scan)
  {
    //while ( *scan && mTypes[*scan] != CT_SOFT ) scan++;
	while ( *scan && !checkTypes(*scan,CT_SOFT) ) scan++;
    return scan;
  }

  inline wchar *nextSoftOrClose(wchar *scan,bool &close)
  {
    //while ( *scan && mTypes[*scan] != CT_SOFT && *scan != '>' ) scan++;
	while ( *scan && !checkTypes(*scan,CT_SOFT) && *scan != '>' ) 
		scan++;
    close = *scan == '>';
    return scan;
  }

  inline wchar *nextSep(wchar *scan)
  {
    //while ( *scan && mTypes[*scan] != CT_SOFT && *scan != '=' ) scan++;
	while ( *scan && !checkTypes(*scan,CT_SOFT) && *scan != '=' ) scan++;
    return scan;
  }

  inline wchar * skipNextData(wchar *scan)
  {
    // while we have data, and we encounter soft seperators or line feeds...
	if(*scan > 256 )
		return scan;

    //while ( *scan && mTypes[*scan] == CT_SOFT || mTypes[*scan] == CT_END_OF_LINE )
	while ( *scan && checkTypes(*scan,CT_SOFT) || checkTypes(*scan,CT_END_OF_LINE) )
    {
      if ( *scan == 13 ) mLineNo++;
      scan++;
    }
    return scan;
  }

  wchar * processClose(wchar c,const wchar *element,wchar *scan, NxI32 argc,const wchar **argv,FastXmlInterface *iface)
  {
    if ( c == '/' || c == '?' )
    {
		wchar *slash = (wchar*)unicode::strchr((wchar*)element,'/');
		if ( slash )
		{
			*slash = 0;
		}
		bool ok = iface->processElement(element,argc,argv,0,mLineNo);
		pushElement(element);
		const wchar *close = popElement();
		iface->processClose(close);
		
		if ( !slash )
			scan++;

		if ( !ok )
		{
			mError = L"User aborted the parsing process";
			return 0;
		}
    }
    else
    {
      scan = skipNextData(scan);
      wchar *data = scan; // this is the data portion of the element, only copies memory if we encounter line feeds
      wchar *dest_data = 0;
      while ( *scan && *scan != '<' )
      {
        //if ( mTypes[*scan] == CT_END_OF_LINE )
		if ( checkTypes(*scan,CT_END_OF_LINE) )
        {
          if ( *scan == 13 ) mLineNo++;
          dest_data = scan;
          *dest_data++ = 32; // replace the linefeed with a space...
          scan = skipNextData(scan);
          while ( *scan && *scan != '<' )
          {
            //if ( mTypes[*scan] == CT_END_OF_LINE )
		    if ( checkTypes(*scan,CT_END_OF_LINE) )
            {
             if ( *scan == 13 ) mLineNo++;
             *dest_data++ = 32; // replace the linefeed with a space...
              scan = skipNextData(scan);
            }
            else
            {
              *dest_data++ = *scan++;
            }
          }
          break;
        }
        else
          scan++;
      }
      if ( *scan == '<' )
      {
        if ( dest_data )
        {
          *dest_data = 0;
        }
        else
        {

			*scan = 0;
        }
        scan++; // skip it..
        if ( *data == 0 ) data = 0;
        bool ok = iface->processElement(element, argc, argv, data, mLineNo);
        pushElement(element);
        if ( !ok )
        {
			mError = L"User aborted the parsing process";
			return 0;
        }
		// check for the comment use case...
		if ( scan[0] == '!' && scan[1] == '-' && scan[2] == '-' )
		{
		  scan+=3;
		  while ( *scan && *scan == 32 )
			  scan++;
		  wchar *comment = scan;
		  wchar *comment_end = (wchar*)unicode::strstr(scan, L"-->");
		  if ( comment_end )
		  {
			  *comment_end = 0;
			  scan = comment_end+3;
			  iface->processComment(comment);
		  }
		}
        else if ( *scan == '/' )
        {
		  scan = processClose(scan,iface);
        }
      }
      else
      {
		mError = L"Data portion of an element wasn't terminated properly";
		return 0;
      }
    }
    return scan;
  }

  wchar * processClose(wchar *scan,FastXmlInterface *iface)
  {
	  const wchar *close = popElement();
	  if ( scan[1] != '>')
	  {
		  scan++;
		  close = scan;
			while ( *scan && *scan != '>' ) scan++;
			*scan = 0;
	  }
	  iface->processClose(close);
	  scan++;
	  return scan;
  }

  virtual bool processXmlFile(const wchar* filename, FastXmlInterface* iface)
  {
	bool ret = true;
	release();

	int numbytes = utility::GetFileLength(filename);

	FILE *infile = NULL;
	_wfopen_s(&infile, filename, L"rb");

	if(infile == NULL)
	return NULL;

	
	mInputData = (char*)malloc((numbytes + 1) * sizeof(char));

	if(mInputData == NULL)
	return NULL;

	memset(mInputData, 0, (numbytes + 1) * sizeof(char));

	fread(mInputData, numbytes, 1, infile);
	fclose(infile);

	int pos = 0;

	if( (unsigned char)mInputData[0] == 0xef && (unsigned char)mInputData[1] == 0xbb && (unsigned char)mInputData[2] == 0xbf )
		pos = 3;

	wchar *dest = new wchar[numbytes];
	memset(dest, 0, numbytes * sizeof(wchar));
	unicode::convstr_s(dest, numbytes, mInputData+pos, true);

	ret =  processXml(iface, dest);
	delete [] dest;

	return ret;
  }

  // input == utf-8
  virtual bool processXml(const wchar *inputData, unsigned int dataLen, FastXmlInterface *iface)
  {
	/*
	wchar *dest = new wchar[dataLen * 2];
	memset(dest, 0, dataLen * 2 * sizeof(wchar));
	int rs = unicode::utf8toutf16(dest, dataLen * 2, (char*)inputData);

	bool ret =  processXml(iface, dest);
	delete [] dest;
	*/

	bool ret =  processXml(iface, (wchar *)inputData);
	return ret;
  }


  bool processXml(FastXmlInterface *iface , wchar *buf)
  {
    bool ret = true;

    #define MAX_ATTRIBUTE 2048 // can't imagine having more than 2,048 attributes in a single element right?
    mLineNo = 1;
    wchar *element;
    //wchar *scan = (wchar*)mInputData;
	wchar *scan = buf;

	while ( *scan )
	{
		scan = skipNextData(scan);
		if ( *scan == 0 ) return ret;
		if ( *scan == '<' )
		{
		  scan++;
		  if ( scan[0] == '!' && scan[1] == '-' && scan[2] == '-' )
		  {
			  scan+=3;
			  while ( *scan && *scan == 32 )
				  scan++;
			  wchar *comment = scan;
			  wchar *comment_end = (wchar *)unicode::strstr(scan, L"-->");
			  if ( comment_end )
			  {
				  *comment_end = 0;
				  scan = comment_end+3;
				  iface->processComment(comment);
			  }
			  continue;
		  }
		}
		if ( *scan == '/' || *scan == '?' )
		{
		  scan = processClose(scan,iface);
		}
		else
		{
		  element = scan;
		  NxI32 argc = 0;
		  const wchar *argv[MAX_ATTRIBUTE];
		  bool close;
		  scan = nextSoftOrClose(scan,close);
		  if ( close )
		  {
			wchar c = *(scan-1);
			if ( c != '?' && c != '/' )
			{
			  c = '>';
			}
			*scan = 0;
			scan++;
			scan = processClose(c,element,scan,argc,argv,iface);
			if ( !scan ) return false;
		  }
		  else
		  {
			if ( *scan == 0 ) return ret;
			*scan = 0; // place a zero byte to indicate the end of the element name...
			scan++;

			while ( *scan )
			{
			  scan = skipNextData(scan); // advance past any soft seperators (tab or space)

			  //if ( mTypes[*scan] == CT_END_OF_ELEMENT )
			  if ( checkTypes(*scan, CT_END_OF_ELEMENT) )
			  {
				wchar c = *scan++;
				scan = processClose(c,element,scan,argc,argv,iface);
				if ( !scan ) return false;
				break;
			  }
			  else
			  {
				if ( argc >= MAX_ATTRIBUTE )
				{
				  mError = L"encountered too many attributes";
				  return false;
				}
				argv[argc] = scan;
				scan = nextSep(scan);  // scan up to a space, or an equal
				if ( *scan )
				{
				  if ( *scan != '=' )
				  {
					*scan = 0;
					scan++;
					while ( *scan && *scan != '=' ) scan++;
					if ( *scan == '=' ) scan++;
				  }
				  else
				  {
					*scan=0;
					scan++;
				  }
				  if ( *scan ) // if not eof...
				  {
					scan = skipNextData(scan);
					if ( *scan == 34 )
					{
					  scan++;
					  argc++;
					  argv[argc] = scan;
					  argc++;
					  while ( *scan && *scan != 34 ) scan++;
					  if ( *scan == 34 )
					  {
						*scan = 0;
						scan++;
					  }
					  else
					  {
						mError = L"Failed to find closing quote for attribute";
						return false;
					  }
					}
					else
					{
					  //mError = "Expected quote to begin attribute";
					  //return false;
					  // PH: let's try to have a more graceful fallback
					  argc--;
					  while (*scan != '/' && *scan != '>' && *scan != 0)
						  scan++;
					}
				  }
				}
			  }
          }
        }
      }
    }

    return ret;
  }

  const wchar * getError(NxI32 &lineno)
  {
    const wchar *ret = mError;
    lineno = mLineNo;
    mError = 0;
    return ret;
  }

  void pushElement(const wchar *element)
  {
  	if ( mStackIndex < MAX_STACK )
  	{
    	mStack[mStackIndex++] = element;
    }
  }

  const wchar * popElement(void)
  {
  	const wchar *ret = 0;
  	if ( mStackIndex )
  	{
  		mStackIndex--;
  		ret = mStack[mStackIndex];
  	}
  	return ret;
  }

private:
  char         mTypes[256];
  char        *mInputData;
  int	       mLineNo;
  const wchar  *mError;
  NxU32        mStackIndex;
  const wchar  *mStack[MAX_STACK];
};

}; // end of namespace

using namespace FAST_XML;

FastXml * createFastXml(void)
{
	MyFastXml *m = new MyFastXml;
	return static_cast< FastXml *>(m);
}

void      releaseFastXml(FastXml *f)
{
	MyFastXml *m = static_cast< MyFastXml *>(f);
	delete m;
}

