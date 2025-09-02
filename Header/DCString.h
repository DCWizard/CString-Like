#pragma once
#ifndef DCSTRING_H
#define DCSTRING_H

  #define SHOW_Version "DCStr"
  #define METHOD "ClearCode TM" // CREATED IN THE 90'S -> ONE GIANT STEP BETTER THAN CLEAN CODE.
  #define THE_CopyRightNotice "`Copyright (c) 2020-2025 Denis Cote ThinknSpeak TM"

#define WIN32_LEAN_AND_MEAN  true

#include <Windows.h>    // FOR HANDLE & CreateMutex
#include <string.h>     // FOR OBVIOUS LOW STRING FUNCTIONS
#include <stdio.h>      // FOR vsprintf_s
#include <malloc.h>     // FOR MALLOCH 
// ---------------- Constants and Macros -----------

  using namespace std;

  typedef char  CHR;
  typedef char* PSTR;
  typedef const char* CSTR;
  typedef unsigned char  BYTE;
  typedef unsigned char* PBYTE;
  typedef unsigned const char* CBYTE;
  #define ReCast(A) reinterpret_cast< A >

  #define null NULL  
  // CLEAN ACCESS TO VAR TYPE
  #define DSTR  DCStr
  #define CDSTR const DCStr
  
// ---------------- class DCStr -----------
  class DCStr {
   public: 
    PSTR          TheStrBuffer        = NULL;
    int           TheLen              = 0;
    int           TheBffSize          = 0;

    #define DCStrAppendZone 32
    #define DCStrEndingZone 8
    #define DCStrBufferZone DCStrAppendZone + DCStrEndingZone
    
    HANDLE        StrMutex            = NULL;
    HANDLE        InitStrMutex    (){
      if(StrMutex  == NULL){
        StrMutex    = CreateMutex(NULL, FALSE, NULL);
      }
      return StrMutex;
    }
    void          DelStrMutex     (){
      if(StrMutex){
        CloseHandle(StrMutex); StrMutex = NULL; 
      }  
    }
    BOOL          AcquireStrMutex (DWORD ThisTimeout = INFINITE){
      if(StrMutex){
        if(WAIT_OBJECT_0 == WaitForSingleObject(StrMutex, ThisTimeout)){
          return true;
        }
      }
      return false;
    }
    BOOL          ReleaseStrMutex (){
      if(StrMutex){
        return ReleaseMutex(StrMutex);
      }
      return false;
    }

    void          InitDCtring     (){
      InitStrMutex();
    } 

    ~DCStr                        () throw(){
      if((TheStrBuffer)){
        free(TheStrBuffer);
        TheStrBuffer        = NULL;
        TheLen              = 0;
        TheBffSize          = 0;
      }
      DelStrMutex();
    }
    DCStr                         () throw(){
      InitDCtring();
      TheBffSize            = DCStrAppendZone + DCStrEndingZone;
      TheStrBuffer          = (PSTR)malloc(TheBffSize);
      ZeroMemory(TheStrBuffer, TheBffSize);
    }
    DCStr         (CSTR ThisStr){
      InitDCtring();
      SetString(ThisStr);
    }


    void          EnterCriticalArea (){
      AcquireStrMutex();
    }
    void          LeaveCriticalArea (){
      ReleaseStrMutex();
    }
    BOOL          Preallocate   (int ThisSize){
      if(ThisSize            <= TheLen){
        return true;
      }
      if(ThisSize            <= (TheLen + DCStrAppendZone)){
        return true;
      }
      EnterCriticalArea();
      TheBffSize              = ThisSize + DCStrBufferZone;
      PSTR TheTmpBuffer       = (PSTR)malloc(TheBffSize);
      ZeroMemory(TheTmpBuffer,  TheBffSize);
      strcpy(TheTmpBuffer,      TheStrBuffer);
      free(TheStrBuffer);
      TheStrBuffer            = TheTmpBuffer;
      LeaveCriticalArea ();
      return true;
    }
    DCStr&        SetString     (CSTR ThisStr){
      // if(0xcccccccc          == ReCast(DWORD)(ThisStr)){
      //   return(*this);
      // }
      EnterCriticalArea();
      if(TheLen               > 0){
        free(TheStrBuffer);
      }
      TheLen                  = strlen(ThisStr);
      TheBffSize              = TheLen + DCStrBufferZone;
      TheStrBuffer            = (PSTR)malloc(TheBffSize);
      ZeroMemory(TheStrBuffer, TheBffSize);
      strcpy(TheStrBuffer, ThisStr);
      LeaveCriticalArea ();
      return(*this);
    }
    DCStr&        Append        (CSTR ThisStr)  throw(){
      if(TheLen              <= 0){
        return SetString        (ThisStr);
      }
      EnterCriticalArea();
      int TheTmpLen           = strlen(ThisStr);
      int TheTltLen           = TheLen + TheTmpLen;
      if(TheTmpLen            > 0){
        Preallocate(TheTltLen);
        strcpy(&TheStrBuffer[TheLen], ThisStr);
        TheLen                = strlen(TheStrBuffer);
      }
      LeaveCriticalArea ();
      return(*this);
    }
    DCStr&        AppendChar    (CHAR ThisChar){
      return Append(&ThisChar);
    }
    int           Insert        (int ThisStart, CSTR ThisInsert){
      if(ThisStart            < 0){
        ThisStart             = 0;
      } else
      if(ThisStart            > TheLen){
        ThisStart             = TheLen;
      }
      int InsertLength        = strlen( ThisInsert );
      int TheNewLen           = TheLen;
      if(InsertLength         > 0){
        TheNewLen            += InsertLength;
        EnterCriticalArea();
        Preallocate(TheNewLen);

        int  StartWhere       = ThisStart;
        int  FinishWhere      = StartWhere + InsertLength;
        int  TheRestLen       = TheLen     - (StartWhere);
        CSTR TheRestStr       = &TheStrBuffer[StartWhere];

        memmove_s(&TheStrBuffer[FinishWhere], TheRestLen,  // WHO'S THE RETARDED IMBECILE WHO CAME UP WITH THAT?
                  TheRestStr, TheRestLen);
        memcpy(&TheStrBuffer[StartWhere],   ThisInsert, InsertLength);
        TheLen                = strlen(TheStrBuffer);

        // CSTR pszBuffer         = GetBuffer( TheNewLen );
        // // move existing bytes up (MOVE THE REST OF THE STR MAKING ROOM FOR THE INSERT)
        // Checked::memmove_s((pszBuffer + ThisStart + InsertLength), 
        //                    (TheNewLen - ThisStart - InsertLength + 1) * sizeof( XCHAR ),  // WHO'S THE RETARDED IMBECILE WHO CAME UP WITH THAT?
        //                    (pszBuffer + ThisStart), 
        //                    (TheNewLen - ThisStart - InsertLength + 1) * sizeof( XCHAR ) 
        //                    );
        // Checked::memcpy_s( (pszBuffer + ThisStart), 
        //                    (InsertLength * sizeof( XCHAR )), 
        //                    (ThisInsert), 
        //                    (InsertLength*sizeof( XCHAR )) 
        //                    );
        // ReleaseBufferSetLength( TheNewLen );
        LeaveCriticalArea ();
      }
      return( TheNewLen );
    }
    int           Insert        (int ThisStart,  CHAR ThisChar){
      return Insert(ThisStart,  &ThisChar);
    }
    DCStr&        Format        (CSTR ThisFormat, ...){
      int       Written;
      char      ThisBuffer    [4096 + DCStrBufferZone];

      va_list   argList;
      va_start( argList, ThisFormat );
      // FormatV( pszFormat, argList );
      Written = vsprintf_s( ThisBuffer, 4096, ThisFormat, argList );
      va_end(   argList );
      return SetString(ThisBuffer);
    }
    static DCStr  Concatenate   (CSTR ThisStr, CSTR ThatStr){
      int TheTm1Len           = strlen(ThisStr);
      int TheTm2Len           = strlen(ThatStr);
      int TheTltLen           = TheTm1Len + TheTm2Len;
      if(TheTm2Len            > 0){
        PSTR TheTmpBuffer     = (PSTR)malloc(TheTltLen + DCStrBufferZone);
        ZeroMemory(TheTmpBuffer, TheTltLen);

        strcpy(TheTmpBuffer,    ThisStr);
        strcpy(&TheTmpBuffer[TheTm1Len], ThatStr);
        DCStr  ThisRetStr     = TheTmpBuffer;
        free(TheTmpBuffer);
        return ThisRetStr;
      }
      return DCStr(ThisStr);
    }
    static int    CompareString (CSTR Str1, CSTR Str2) throw(){
      return strcmp( (Str1), (Str2) );
      // return _mbscmp( ReCast(CBYTE)(Str1), ReCast(CBYTE)(Str2) );
    }
    static int    CompareNoCase (CSTR Str1, CSTR Str2) throw(){
      return strcmpi( (Str1), (Str2) );
      // return _mbsicmp( ReCast(CBYTE)(Str1), ReCast(CBYTE)(Str2) );
    }
    int           Compare       (CSTR ThisStr){
      return CompareString(TheStrBuffer, ThisStr);
    }
    CSTR          GetBuffer     (){
      return TheStrBuffer;
    }
    int           GetBufferLen  () const throw(){  // MEASURE UPT0 DCStrAppendZone - DCStrEndingZone
      return( TheBffSize - DCStrEndingZone );
    }
    int           GetLength     (){
      return TheLen;
    }
    bool          IsEmpty       (){
      return (TheLen         == 0);
    }
    void          Empty         (){
      if(TheLen              == 0){
        return;
      }
      ZeroMemory(TheStrBuffer, TheBffSize);
      TheLen                  = 0;
    }
    void          Truncate      (int ThisNewLen){
      if(TheLen               > ThisNewLen){
        TheLen                = ThisNewLen;
        int ThisGap           = TheBffSize - TheLen;
        ZeroMemory(&TheStrBuffer[TheLen], ThisGap);
      }
    }
    char          GetAt         (int Indx){
      if((Indx                < 0)
      || (Indx               >= TheLen)
      ){
        return NULL;
      }
      return      TheStrBuffer[Indx];
    }
    BOOL          SetAt         (int Indx, CHAR ThisChar){
      if((Indx                < 0)
      || (Indx               >= TheLen)
      ){
        return false;
      }
      TheStrBuffer[Indx]      = ThisChar;
      TheLen                  = 0;
    }
    DCStr         Make          (CSTR ThisStr,   int ThisLen = NULL){
      if(ThisLen             == NULL){
        ThisLen               = strlen(ThisStr);
      }
      PSTR ThisTempStr        = (PSTR)malloc(ThisLen + DCStrBufferZone);
      ZeroMemory(ThisTempStr,   ThisLen + DCStrBufferZone);
      strncpy(ThisTempStr,      ThisStr, ThisLen);
      DCStr ThisOutput        = ThisTempStr;
      free(ThisTempStr);
      return ThisOutput;
    }
    DCStr         Left          (int Indx){
      if((Indx                < 0)
      ){
        return DCStr("");
      }
      if((Indx               >= TheLen)
      ){
        return DCStr(TheStrBuffer);
      }
      return Make(TheStrBuffer, Indx);
    }
    DCStr         Right         (int Indx){
      if((Indx                < 0)
      ){
        return DCStr("");
      }
      if((Indx               >= TheLen)
      ){
        return DCStr(TheStrBuffer);
      }
      int TheStart            = TheLen - Indx;
      return Make(&TheStrBuffer[TheStart]);
    }
    DCStr         Mid           (int ThisStart,  int ThisLen = NULL){
      if(ThisLen             == NULL){
        return Right(TheLen   - ThisStart);
      }
      if((ThisStart           < 0)
      ){
        ThisStart             = 0;
      } else 
      if((ThisStart          >= TheLen)
      ){
        return DCStr("");
      }
      // MAKING THE TEMPSTR:
      return Make(&TheStrBuffer[ThisStart], ThisLen);
    }
    int           Find          (CSTR ThisMatch, int ThisStart = 0){
      if((ThisStart           < 0)
      ){
        ThisStart             = 0;
      } else 
      if((ThisStart          >= TheLen)
      ){
        return -1;
      }
      CSTR ThisRet            =  strstr(&TheStrBuffer[ThisStart], ThisMatch);
      return( (ThisRet == NULL) ? -1 : int( ThisRet - TheStrBuffer ) );
    }
    int           Find          (CHAR ThisMatch, int ThisStart = 0){
      if((ThisStart           < 0)
      ){
        ThisStart             = 0;
      } else 
      if((ThisStart          >= TheLen)
      ){
        return -1;
      }
      CSTR ThisRet            = strchr(&TheStrBuffer[ThisStart], ThisMatch);
      return( (ThisRet == NULL) ? -1 : int( ThisRet - TheStrBuffer ) );
    }
    int           ReverseFind   (CHAR ThisMatch, int ThisStart = 0){
      if((ThisStart           < 0)
      ){
        ThisStart             = 0;
      } else 
      if((ThisStart          >= TheLen)
      ){
        return -1;
      }
      CSTR ThisRet            = strrchr(&TheStrBuffer[ThisStart], ThisMatch);
      return( (ThisRet == NULL) ? -1 : int( ThisRet - TheStrBuffer ) );
    }

    // RETURN THE NUMBER OF CHANGE OCCURENCE OF CHANGE WITHIN THE TheStrBuffer
    int           Replace       (CSTR ThisMatch, CSTR ThisReplace){
      CSTR ThisFound;
      int  ThisStart          = 0;
      int  FoundCount         = 0;
      int  MatchLen           = strlen(ThisMatch);
      int  ReplaceLen         = strlen(ThisReplace);
      int  ReplaceDiff        = ReplaceLen - MatchLen;
      EnterCriticalArea();
      do{
        ThisFound             = strstr(&TheStrBuffer[ThisStart], ThisMatch);
        if(ThisFound){
          FoundCount++;
          int  StartWhere     = ThisFound  - TheStrBuffer;
          int  FinishWhere    = StartWhere + ReplaceLen;
          int  TheRestLen     = TheLen     - (StartWhere + MatchLen);
          CSTR TheRestStr     = &TheStrBuffer[StartWhere + MatchLen];

          if(ReplaceDiff      > 0){ // WE'RE GONNA NEED MORE BUFFER
            // REBUILD STR BUFFER WITH NEW SIZE/LEN
            int TheNewLen     = (TheLen + ReplaceDiff);
            Preallocate(TheNewLen);

            memmove_s(&TheStrBuffer[FinishWhere],  // WHO'S THE RETARDED IMBECILE WHO CAME UP WITH THAT?
                      TheRestLen, TheRestStr, TheRestLen);
            memcpy (&TheStrBuffer[StartWhere],  ThisReplace, ReplaceLen);
            TheLen            = strlen(TheStrBuffer);
          } else { //EASY
            // REBUILD STR BUFFER WITH NEW LEN ONLY IF NECESSARY
            memcpy(&TheStrBuffer[StartWhere],  ThisReplace, ReplaceLen);
            strcpy(&TheStrBuffer[FinishWhere], TheRestStr);
            TheLen            = strlen(TheStrBuffer);
          }
          ThisStart           = FinishWhere;
        }
      } while(ThisFound);
      LeaveCriticalArea ();
      return FoundCount;
    }
    int           Replace       (CHAR ThisMatch, CHAR ThisReplace){
      return Replace       (&ThisMatch, &ThisReplace);
    }
    DCStr         Tokenize      (CSTR ThisToken, int& ThisStart){
      if(ThisStart            < 0){
        ThisStart             = 0;
      }
      if((ThisToken          == NULL) 
      || (*ThisToken         == (CHAR)0) 
      ){
        if(ThisStart          < TheLen){
          return( DCStr(TheStrBuffer + ThisStart) );
        }
      }  else {
        CSTR StartStr         = TheStrBuffer + ThisStart;
        CSTR EndStr           = TheStrBuffer + TheLen;
        if(StartStr           < EndStr ){
          int Including       = strspn( StartStr, ThisToken );

          if(EndStr          >= (StartStr    + Including)){
            StartStr         += Including;
            int Excluding     = strcspn( StartStr, ThisToken );
            int ThisFrom      = ThisStart    + Including;
            int UntilTo       = Excluding;
            DCStr ThisRet     = Mid( ThisFrom, UntilTo );
            ThisStart         = ThisFrom + UntilTo + 1;

            return(ThisRet);
          }
        }
      }
      // return empty string, done tokenizing
      ThisStart               = -1;
      DCStr ThisRet           = Make("");
      return( ThisRet );
    }
    int           Delete        (int Index, int ThisCount = 1){
      if(Index                < 0 ){
        Index = 0;
      } else 
      if(Index               >= TheLen){
        return TheLen;
      }
      if(ThisCount           <= 0){
        return TheLen;
      }
      if((Index + ThisCount)  > TheLen){
        ThisCount             = (TheLen - Index);
      }
      int NewLen              = TheLen  - ThisCount;
      int MoveLen             = TheLen  - (Index + ThisCount) + 1;
      memmove_s(                (&TheStrBuffer[Index]), 
                                (MoveLen), // WHO'S THE RETARDED IMBECILE WHO CAME UP WITH THAT?  
                                (&TheStrBuffer[Index + ThisCount]), 
                                (MoveLen) 
                );
      TheLen                  = strlen(TheStrBuffer);
      return(TheLen);
    }
    // Remove all occurrences of character 'RemoveChar'
    int           Remove        (CHAR RemoveChar){
      PSTR CurSrc             = TheStrBuffer;
      PSTR CurDst             = TheStrBuffer;
      PSTR EndStr             = TheStrBuffer + TheLen;

      while(CurSrc            < EndStr ){
        PSTR NewSrc           = CurSrc  + 1;
        if(*CurSrc           != RemoveChar){
          int NewSrcGap       = (NewSrc - CurSrc);
          PSTR NewDst         = CurDst  + NewSrcGap;
          for(int Indx = 0; (CurDst != NewDst) && (Indx < NewSrcGap); Indx++){
            *CurDst           = *CurSrc;
            CurSrc++;
            CurDst++;
          }
        }
        CurSrc                = NewSrc;
      }
      *CurDst                 = 0;
      int ThisNewLen          = int( CurSrc - CurDst );
      TheLen                  = strlen(TheStrBuffer);   // THEY ARE SUPPOSE TO BE THE SAME. I PREFER TO CHECK
      return(TheLen);
    }
    DCStr&        TrimLeft      (CSTR ThisTarget = " "){
      if((ThisTarget         == NULL) 
      || (*ThisTarget        == 0) 
      ){
        return( *this );
      }
      CSTR Pez                = TheStrBuffer; // Pez dispenser.
      while((*Pez            != 0) 
      &&    (NULL            != strchr(ThisTarget, *Pez )) 
      ){
        Pez++;
      }
      if(Pez                 != TheStrBuffer ){
        int ISkip             = int( Pez - TheStrBuffer );
        int TheNewLen         = TheLen   - ISkip;
        memmove_s(              TheStrBuffer, 
                                (TheNewLen + 1),  // WHO'S THE RETARDED MORON WHO CAME UP WITH THAT IDEA?  
                                Pez, 
                                (TheNewLen + 1) 
                  );
        TheLen                = strlen(TheStrBuffer);    
      }
      return( *this );
    }
    DCStr&        TrimLeft      (CHAR ThisTarget){
      return TrimLeft(&ThisTarget);
    }
    DCStr&        TrimRight     (CSTR ThisTarget = " "){
      if((ThisTarget         == NULL) 
      || (*ThisTarget        == 0) 
      ){
        return(*this);
      }
      CSTR Pez                = TheStrBuffer; // Pez dispenser.
      CSTR LastPart           = NULL;

      while(*Pez             != 0){
        if(NULL              != strchr(ThisTarget, *Pez)){
          if(LastPart        == NULL){
            LastPart          = Pez;
          }
        } else {
          LastPart            = NULL;
        }
        Pez++;
      }
      if(LastPart            != NULL ){
        int ISkip             = int(LastPart - TheStrBuffer);
        Truncate( ISkip );
      }
      return(*this);
    }
    DCStr&        TrimRight     (CHAR ThisTarget){
      return TrimRight(&ThisTarget);
    }
    DCStr&        Trim          (CSTR ThisTarget = " "){
      return( TrimRight( ThisTarget ).TrimLeft( ThisTarget ) );
    }
    DCStr&        Trim          (CHAR ThisTarget){
      return( TrimRight( &ThisTarget ).TrimLeft( &ThisTarget ) );
    }
    // +

    DCStr         Copy          (CSTR ThisStr){
      return SetString(ThisStr);
    }
   // MAIN CAST
    operator      CSTR          ()const throw(){
      return(TheStrBuffer);
    }

    // When is the Copy Constructor Called?
      // In C++, a copy constructor may be called in the following cases: 

      // When an object of the class is returned by value.
      // When an object of the class is passed (to a function) by value as an argument.
      // When an object is constructed based on another object of the same class.
      // When the compiler generates a temporary object.
    DCStr(CDSTR& ThisStr){ // {TheStrBuffer=0xcccccccc <Error reading characters of string.> TheLen=-858993460 TheBffSize=-858993460 ...}
      SetString(ThisStr.TheStrBuffer);
    }

    DCStr&        operator=     (CDSTR& ThisStr){
      // KEPT IN COMMENT FOR REMEMBERING THAT SHIT!
      // THE RETURN IS IRRELEVANT...
      // WHAT MATTERS IS WHO'S MAKING THE COPY OF WHAT!
      // IN THIS CASE IT'S THE RECIEVER'S.

      // return DCStr(ThisStr.TheStrBuffer);
      // return( *this );
      return SetString(ThisStr);
    }  
    DCStr&        operator=     (CSTR ThisStr){
      return SetString(ThisStr);
    }  
    CHR           operator[]    (int Indx){
      return TheStrBuffer[Indx];
    }


   // OPERATORS
    friend DCStr  operator+     (CDSTR& Str1, CDSTR& Str2){
      return Concatenate( Str1, Str2 );
    }
    friend DCStr  operator+     (CDSTR& Str1, CSTR Str2){
      return Concatenate( Str1, Str2 );
    }
    friend DCStr  operator+     (CSTR Str1,   CDSTR& Str2){
      return Concatenate( Str1, Str2 );
    }
    friend DCStr  operator+     (CDSTR& Str1, CHAR Chr2){
      return Concatenate( Str1, &Chr2 );
    }
    friend DCStr  operator+     (CHAR Chr1,   CDSTR& Str2){
      return Concatenate( &Chr1, Str2 );
    }
    DCStr&        operator+=    (CDSTR ThisStr){
      return Append(ThisStr);
    }
    DCStr&        operator+=    (CSTR  ThisStr){
      return Append(ThisStr);
    }
    DCStr&        operator+=    (CHAR  ThisChar){
      return AppendChar(ThisChar);
    }


   // COMPARE
    friend bool   operator==    (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) == 0 );
    }
    friend bool   operator==    (CDSTR& Str1,  CSTR Str2)    throw(){
      return(CompareString(Str1, Str2) == 0 );
    }
    friend bool   operator==    (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) == 0 );
    }

    friend bool   operator!=    (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) != 0 );
    }
    friend bool   operator!=    (CDSTR& Str1,  CSTR Str2)    throw(){
      return(CompareString(Str1, Str2) != 0 );
    }
    friend bool   operator!=    (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) != 0 );
    }

    friend bool   operator>=    (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) >= 0 );
    }
    friend bool   operator>=    (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2) >= 0 );
    }
    friend bool   operator>=    (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) >= 0 );
    }
      
    friend bool   operator<=    (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) <= 0 );
    }
    friend bool   operator<=    (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2) <= 0 );
    }
    friend bool   operator<=    (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) <= 0 );
    }
    
    friend bool   operator>     (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2)  > 0 );
    }
    friend bool   operator>     (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2)  > 0 );
    }
    friend bool   operator>     (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2)  > 0 );
    }
    
    friend bool   operator<     (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2)  < 0 );
    }
    friend bool   operator<     (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2)  < 0 );
    }
    friend bool   operator<     (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2)  < 0 );
    }
  }; 
#endif // DCSTRING_H
