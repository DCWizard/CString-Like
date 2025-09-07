// DCString.h
#pragma once
// #define _CRT_SECURE_NO_WARNINGS
// #define _AFX_SECURE_NO_WARNINGS
// #define _CRT_SECURE_NO_WARNINGS_GLOBALS
// #define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable : 4996)

#ifndef DCSTRING_H
#define DCSTRING_H

  #define SHOW_Version "DCStr"
  #define METHOD "ClearCode TM" // CREATED IN THE 90'S -> ONE GIANT STEP BETTER THAN CLEAN CODE.
  #define THE_CopyRightNotice "`Copyright (c) 2020-2025 Denis Cote ThinknSpeak TM"

#define WIN32_LEAN_AND_MEAN  true

#ifndef _WINDOWS_
  #include <Windows.h>    // FOR HANDLE & CreateMutex
#endif
  union MultiVal {
    __int64   i64; 
    double    Dbl;
    float     Fl;
    unsigned  Un; 
    int       Int;
    char      Char;
  };

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

// -------------------- <Memory Mngr> --------------
  int     SlotCount          = 0;
  bool    SlotInited         = false;
  #define SlotSize             4096
  void*   MemorySlot[SlotSize];  
  void*   AllocSDCMem          (int HowMuch){
    if(!(SlotInited)){
      ZeroMemory(MemorySlot, sizeof(MemorySlot));
      SlotInited             = true;
    }
    if(SlotCount            >= SlotSize - 2){
      perror("Not Enough Mem slot.");
      throw("Not Enough Mem slot.");
      return null;
    }

    void* ThisBuffer         = malloc(HowMuch); 
    // CHECK IF PTR ALREADY IN USE
    for(int Indx=0; Indx     < (SlotSize - 1); Indx++){
      if(MemorySlot[Indx]   == ThisBuffer){
        perror("Re-Allocating the same block without having been freed???");
        MemorySlot[Indx]     = 0;
        SlotCount++;
        return ThisBuffer;
      }
    }
    // FIND A FREE SLOT. + STORE PTR
    bool PlacedMem           = false;
    for(int Indx = 0;Indx    < (SlotSize - 1); Indx++){
      if(MemorySlot[Indx]   == 0){
        MemorySlot[Indx]     = ThisBuffer;
        PlacedMem            = true; 
        break;
      }
    }
    if(!(PlacedMem)){
      perror("Could not find a place to put new Memory alloc.");
    }
    return ThisBuffer;
  }
  void    FreeSDCMem           (void* WhichOne){
    bool FoundMem            = false;
    for(int Indx=0; Indx     < (SlotSize - 1); Indx++){
      if(MemorySlot[Indx]   == WhichOne){
        MemorySlot[Indx]     = 0;
        free(WhichOne);
        FoundMem             = true;
        SlotCount--;
        return;
      }
    }
    // MEMORY NOT FOUND ALREADY DELETED
    perror("Multiple block freeing.");
    throw("Multiple block freeing.");
    return;
  }
// -------------------- class DCStr ----------------
  class DCStr {
   public: 
    PSTR          TheStrBuffer    = NULL;
    int           TheLen          = 0;
    int           TheBffSize      = 0;
    bool          EnableMutex     = false;

    #define DCStrAppendZone 32
    #define DCStrEndingZone 8
    #define DCStrBufferZone DCStrAppendZone + DCStrEndingZone
    #define DCStrUseMutex false
    
    HANDLE        StrMutex        = NULL;

    HANDLE        InitStrMutex      (){
      if(!(DCStrUseMutex)){
        return NULL;
      }
      if(StrMutex                  == NULL){
        StrMutex                    = CreateMutex(NULL, FALSE, NULL);
      }
      return StrMutex;
    }
    void          DelStrMutex       (){
      if(!(DCStrUseMutex)){
        return;
      }
      if(StrMutex){
        CloseHandle(StrMutex); StrMutex = NULL; 
      }  
    }
    BOOL          AcquireStrMutex   (DWORD ThisTimeout = INFINITE){
      if(!(DCStrUseMutex)){
        return true;
      }
      if(StrMutex){
        if(WAIT_OBJECT_0 == WaitForSingleObject(StrMutex, ThisTimeout)){
          return true;
        }
      }
      return false;
    }
    BOOL          ReleaseStrMutex   (){
      if(StrMutex){
        return ReleaseMutex(StrMutex);
      }
      return false;
    }

    void          EnterCriticalArea (){
      if(EnableMutex){
        AcquireStrMutex();
      }
    }
    void          LeaveCriticalArea (){
      if(EnableMutex){
        ReleaseStrMutex();
      }
    }
    void          SetAccess         (bool ThisAccess){
      EnableMutex = ThisAccess;
    }


    void          InitDCtring       (){
      InitStrMutex();
    } 

    ~DCStr                          () throw(){
      if((TheStrBuffer)){
        free(TheStrBuffer);
        TheStrBuffer              = NULL;
        TheLen                    = 0;
        TheBffSize                = 0;
      }
      DelStrMutex();
    }
    DCStr                           () throw(){
      InitDCtring();
      TheBffSize                  = DCStrAppendZone + DCStrEndingZone;
      TheStrBuffer                = (PSTR)malloc(TheBffSize);
      ZeroMemory(TheStrBuffer,      TheBffSize);
    }
    DCStr                           (CSTR ThisStr){
      InitDCtring();
      SetString(ThisStr);
    }

    BOOL          Preallocate       (int ThisSize){

      if((ThisSize)              <= (TheBffSize - DCStrEndingZone)){
        return true;
      }

      EnterCriticalArea();
      TheBffSize                  = ThisSize + DCStrBufferZone;
      PSTR TheTmpBuffer           = (PSTR)malloc(TheBffSize);
      ZeroMemory(TheTmpBuffer,      TheBffSize);

      strcpy(TheTmpBuffer,          TheStrBuffer);

      free(TheStrBuffer);
      TheStrBuffer                = TheTmpBuffer;
      LeaveCriticalArea ();
      return true;
    }

    DCStr&        SetString         (CSTR ThisStr){
      if(0xcccccccc              == ReCast(unsigned)(ThisStr)){
        perror("Memory Error in SetString."); 
        return(*this);
      }

      EnterCriticalArea();
      if((TheStrBuffer           != NULL) 
      ){
        free(TheStrBuffer);
        TheStrBuffer              = NULL;
        TheLen                    = 0;
        TheBffSize                = 0;      
      }
      if(0                       == (void*)(ThisStr)){
        TheLen                    = 0;
      } else {
        TheLen                    = strlen(ThisStr);
      }     
      TheBffSize                  = TheLen + DCStrBufferZone;
      TheStrBuffer                = (PSTR)malloc(TheBffSize);
      ZeroMemory(TheStrBuffer,      TheBffSize);
      if(TheLen                   > 0){
        strcpy(TheStrBuffer,        ThisStr);
      }
      LeaveCriticalArea ();
      return(*this);
    }
    DCStr&        Append            (CSTR ThisStr, int ThisSize = NULL)  throw(){
      EnterCriticalArea();
      int TheTmpLen               = strlen(ThisStr);
      int TheTltLen               = TheLen + TheTmpLen;
      if(TheTmpLen                > 0){
        Preallocate(TheTltLen);
        strcpy(&TheStrBuffer[TheLen], ThisStr);
        TheLen                    = strlen(TheStrBuffer);
      }
      LeaveCriticalArea ();
      return(*this);
    }
    DCStr&        AppendChar      (CHAR ThisChar){
      char buf[2]               = { ThisChar, 0 };
      return Append(buf);
    }
    int           Insert          (int ThisStart, CSTR ThisInsert){
      if(!(ThisInsert)){ 
        return TheLen;
      }
      if(ThisStart             <= 0){
        return TheLen;
      } else
      if(ThisStart             >= TheLen){
        return TheLen;
      }
      int InsertLength          = strlen( ThisInsert );
      int TheNewLen             = TheLen;
      if(InsertLength           > 0){
        TheNewLen                += InsertLength;
        
        EnterCriticalArea();
        Preallocate(TheNewLen);

        int  StartWhere           = ThisStart;
        int  FinishWhere          = StartWhere + InsertLength;
        int  TheRestLen           = TheLen     - (StartWhere);
        CSTR TheRestStr           = &TheStrBuffer[StartWhere];

        memmove(&TheStrBuffer[FinishWhere], TheRestStr, TheRestLen);
        memcpy( &TheStrBuffer[StartWhere],  ThisInsert, InsertLength);
        TheLen                = strlen(TheStrBuffer);
        LeaveCriticalArea ();
      }
      return( TheNewLen );
    }
    int           Insert          (int ThisStart,  CHAR ThisChar){
      char buf[2]               = { ThisChar, 0 };
      return Insert(ThisStart,    buf);
    }
    DCStr&        Format          (CSTR ThisFormat, ...){
      int       Written;
      char      ThisBuffer        [4096 + DCStrBufferZone];

      va_list   argList;
      va_start( argList, ThisFormat );
      Written = vsnprintf(ThisBuffer, 4096, ThisFormat, argList);
      va_end(   argList );
      return SetString(ThisBuffer);
    }
    static DCStr  Concatenate     (CSTR ThisStr, CSTR ThatStr){
      int TheTm1Len             = strlen(ThisStr);
      int TheTm2Len             = strlen(ThatStr);
      int TheTltLen             = TheTm1Len + TheTm2Len;
      if(TheTm2Len              > 0){
        PSTR TheTmpBuffer       = (PSTR)malloc(TheTltLen + DCStrBufferZone);
        ZeroMemory(TheTmpBuffer,  TheTltLen + DCStrBufferZone);

        strcpy(TheTmpBuffer,      ThisStr);
        strcpy(&TheTmpBuffer[TheTm1Len], ThatStr);

        DCStr  ThisRetStr       = TheTmpBuffer;
        free(TheTmpBuffer);
        return ThisRetStr;
      }
      return DCStr(ThisStr);
    }
    static int    CompareString   (CSTR Str1, CSTR Str2) throw(){
      return strcmp( Str1, Str2 );
    }
    static int    CompareNoCase   (CSTR Str1, CSTR Str2) throw(){
      return _strcmpi( Str1, Str2 );
    }
    int           CompareNoCase   (CSTR Str2) throw(){
      return CompareNoCase( TheStrBuffer, Str2 );
    }
    int           Compare         (CSTR ThisStr){
      return CompareString(TheStrBuffer, ThisStr);
    }
    CSTR          GetBuffer       (){
      return TheStrBuffer;
    }   
    int           GetBufferLen    () const throw(){  // MEASURE UP T0 DCStrAppendZone - DCStrEndingZone
      return( TheBffSize - DCStrEndingZone );
    }   
    int           GetLength       () const throw(){
      return TheLen;
    }   
    bool          IsEmpty         () const throw(){
      return (TheLen           == 0);
    }   
    void          Empty           (){
      if(TheLen                == 0){
        return;
      }
      ZeroMemory(TheStrBuffer,    TheBffSize);
      TheLen                    = 0;
    }   
    void          Truncate        (int ThisNewLen){
      if(TheLen                 > ThisNewLen){
        TheLen                  = ThisNewLen;
        int ThisGap             = TheBffSize - TheLen;
        ZeroMemory(&TheStrBuffer[TheLen], ThisGap);
      }
    }   
    char          GetAt           (int Indx){
      if((Indx                  < 0)
      || (Indx                 >= TheLen)
      ){
        return NULL;
      }
      return      TheStrBuffer[Indx];
    }   
    BOOL          SetAt           (int Indx, CHAR ThisChar){
      if((Indx                  < 0)
      || (Indx                 >= TheLen)
      ){
        return false;
      }
      TheStrBuffer[Indx]        = ThisChar;
      return true;
    }
    DCStr         Make            (CSTR ThisStr,   int ThisLen = -1) const throw(){
      if(ThisLen               == -1){
        ThisLen                 = strlen(ThisStr);
      }
      PSTR ThisTempStr          = (PSTR)malloc(ThisLen + DCStrBufferZone);
      ZeroMemory(ThisTempStr,     ThisLen + DCStrBufferZone);
      if(ThisLen                > 0){
        strncpy(ThisTempStr,      ThisStr, ThisLen);
      }
      DCStr ThisOutput          = ThisTempStr;
      free(ThisTempStr);
      return ThisOutput;
    }   
    DCStr         Left            (int Indx) const throw(){
      if((Indx                 <= 0)
      ){
        return DCStr("");
      }
      if((Indx                 >= TheLen)
      ){
        return DCStr(TheStrBuffer);
      }
      return Make(TheStrBuffer, Indx);
    }   
    DCStr         Right           (int Indx) const throw(){
      if((Indx                 <= 0)
      ){
        return DCStr("");
      }
      if((Indx                 >= TheLen)
      ){
        return DCStr(TheStrBuffer);
      }
      int TheStart              = TheLen - Indx;
      return Make(&TheStrBuffer[TheStart]);
    }   
    DCStr         Mid             (int ThisStart,  int ThisLen   = -1) const throw(){
      if(ThisLen               <= -1){
        return Right(TheLen     - ThisStart);
      }
      if((ThisStart             < 0)
      ){
        ThisStart               = 0;
      } else 
      if((ThisStart            >= TheLen)
      ){
        return DCStr("");
      }
      // MAKING THE TEMPSTR:
      return Make(&TheStrBuffer[ThisStart], ThisLen);
    }   
    int           Find            (CSTR ThisMatch, int ThisStart = 0) const throw(){
      if((ThisStart             < 0)
      ){
        ThisStart               = 0;
      } else 
      if((ThisStart            >= TheLen)
      ){
        return -1;
      }
      CSTR ThisRet              =  strstr(&TheStrBuffer[ThisStart], ThisMatch);
      return( (ThisRet == NULL) ? -1 : int( ThisRet - TheStrBuffer ) );
    }   
    int           Find            (CHAR ThisMatch, int ThisStart = 0) const throw(){
      if((ThisStart             < 0)
      ){
        ThisStart               = 0;
      } else 
      if((ThisStart            >= TheLen)
      ){
        return -1;
      }
      CSTR ThisRet            = strchr(&TheStrBuffer[ThisStart], ThisMatch);
      return( (ThisRet == NULL) ? -1 : int( ThisRet - TheStrBuffer ) );
    }   
    int           ReverseFind     (CHAR ThisMatch, int ThisStart = 0){
      if((ThisStart             < 0)
      ){
        ThisStart               = 0;
      } else 
      if((ThisStart            >= TheLen)
      ){
        return -1;
      }
      CSTR ThisRet              = strrchr(&TheStrBuffer[ThisStart], ThisMatch);
      return( (ThisRet == NULL) ? -1 : int( ThisRet - TheStrBuffer ) );
    }
    // RETURN THE NUMBER OF CHANGE OCCURENCE OF CHANGE WITHIN THE TheStrBuffer
    int           Replace         (CSTR ThisMatch, CSTR ThisReplace){
      CSTR ThisFound;
      int  ThisStart            = 0;
      int  FoundCount           = 0;
      int  MatchLen             = strlen(ThisMatch);
      int  ReplaceLen           = strlen(ThisReplace);
      int  ReplaceDiff          = ReplaceLen - MatchLen;
      if(MatchLen              <= 0){ 
        return 0;
      }

      EnterCriticalArea();
      do{
        ThisFound               = strstr(&TheStrBuffer[ThisStart], ThisMatch);
        if(ThisFound){
          FoundCount++;
          int  StartWhere       = int(ThisFound - TheStrBuffer);
          int  FinishWhere      = StartWhere    + ReplaceLen;
          int  TheRestLen       = (TheLen       - (StartWhere + MatchLen)) + 1;

          if(ReplaceDiff        > 0){ // WE'RE GONNA NEED MORE BUFFER
            int TheNewLen       = (TheLen + ReplaceDiff);
            Preallocate(TheNewLen); // THE BUFFER MAY MOVE
            
            CSTR TheRestStr     = &TheStrBuffer[StartWhere + MatchLen];

            memmove(&TheStrBuffer[FinishWhere], TheRestStr,  TheRestLen);
            memcpy (&TheStrBuffer[StartWhere],  ThisReplace, ReplaceLen);
            TheLen              = strlen(TheStrBuffer);
          } else { //EASY
            // REBUILD STR BUFFER WITH NEW LEN ONLY IF NECESSARY
            CSTR TheRestStr     = &TheStrBuffer[StartWhere + MatchLen];

            memcpy(&TheStrBuffer[StartWhere],  ThisReplace, ReplaceLen);
            strcpy(&TheStrBuffer[FinishWhere], TheRestStr);
            TheLen              = strlen(TheStrBuffer);
          }
          ThisStart             = FinishWhere;
        }
      } while(ThisFound);
      LeaveCriticalArea ();
      return FoundCount;
    }   
    int           Replace         (CHAR ThisMatch, CHAR ThisReplace){
      char TheMatch[2]          = { ThisMatch, 0 };
      char TheReplace[2]        = { ThisReplace, 0 };
      return Replace(TheMatch,      TheReplace);
    }   
    DCStr         Tokenize        (CSTR ThisToken, int& ThisStart){
      if(ThisStart              < 0){
        ThisStart               = 0;
      }
      if((ThisToken            == NULL) 
      || (*ThisToken           == (CHAR)0) 
      ){
        if(ThisStart            < TheLen){
          return( DCStr(TheStrBuffer + ThisStart) );
        }
      }  else {
        CSTR StartStr           = TheStrBuffer + ThisStart;
        CSTR EndStr             = TheStrBuffer + TheLen;
        if(StartStr             < EndStr ){
          int Including         = (int)strspn( StartStr, ThisToken );

          if(EndStr            >= (StartStr    + Including)){
            StartStr           += Including;
            int Excluding       = (int)strcspn( StartStr, ThisToken );
            int ThisFrom        = ThisStart    + Including;
            int UntilTo         = Excluding;
            DCStr ThisRet       = Mid( ThisFrom, UntilTo );
            ThisStart           = ThisFrom + UntilTo + 1;

            return(ThisRet);
          }
        }
      }
      // return empty string, done tokenizing
      ThisStart                 = -1;
      DCStr ThisRet             = Make("");
      return( ThisRet );
    }   
    int           Delete          (int Index, int ThisCount = 1){
      if(Index                  < 0 ){
        Index = 0;
      } else 
      if(Index                 >= TheLen){
        return TheLen;
      }
      if(ThisCount             <= 0){
        return TheLen;
      }
      if((Index + ThisCount)    > TheLen){
        ThisCount               = (TheLen - Index);
      }
      int NewLen                = TheLen  - ThisCount;
      int MoveLen               = TheLen  - (Index + ThisCount) + 1; // include null terminator
      memmove(                    (&TheStrBuffer[Index]), 
                                  (&TheStrBuffer[Index + ThisCount]), 
                                  (MoveLen) 
                );
      TheLen                    = strlen(TheStrBuffer);
      return(TheLen);
    }
    // Remove all occurrences of character 'RemoveChar'
    int           Remove          (CHAR RemoveChar){
      PSTR CurSrc               = TheStrBuffer;
      PSTR CurDst               = TheStrBuffer;
      PSTR EndStr               = TheStrBuffer + TheLen;

      while(CurSrc              < EndStr ){
        PSTR NewSrc             = CurSrc  + 1;
        if(*CurSrc             != RemoveChar){
          int NewSrcGap         = int(NewSrc - CurSrc);
          PSTR NewDst           = CurDst  + NewSrcGap;
          for(int Indx = 0; (CurDst != NewDst) && (Indx < NewSrcGap); Indx++){
            *CurDst             = *CurSrc;
            CurSrc++;
            CurDst++;
          }
        }
        CurSrc                  = NewSrc;
      }
      *CurDst                   = 0;
      int ThisNewLen            = int( CurSrc - CurDst );
      TheLen                    = strlen(TheStrBuffer);   // THEY ARE SUPPOSE TO BE THE SAME. I PREFER TO CHECK
      return(TheLen);
    }   
    DCStr&        TrimLeft        (CSTR ThisTarget = " "){
      if((ThisTarget           == NULL) 
      || (*ThisTarget          == 0) 
      ){
        return( *this );
      }
      CSTR Pez                  = TheStrBuffer; // Pez dispenser.
      while((*Pez              != 0) 
      &&    (NULL              != strchr(ThisTarget, *Pez )) 
      ){
        Pez++;
      }
      if(Pez                   != TheStrBuffer ){
        int ISkip               = int( Pez - TheStrBuffer );
        int TheNewLen           = TheLen   - ISkip;
        memmove(TheStrBuffer,     Pez, (TheNewLen + 1));
        TheLen                  = strlen(TheStrBuffer);    
      }
      return( *this );
    }   
    DCStr&        TrimLeft        (CHAR ThisTarget){
      char TheTarget[2]         = { ThisTarget, 0 };
      return TrimLeft(TheTarget);
    }   
    DCStr&        TrimRight       (CSTR ThisTarget = " "){
      if((ThisTarget           == NULL) 
      || (*ThisTarget          == 0) 
      ){
        return(*this);
      }
      CSTR Pez                  = TheStrBuffer; // Pez dispenser.
      CSTR LastPart             = NULL;

      while(*Pez               != 0){
        if(NULL                != strchr(ThisTarget, *Pez)){
          if(LastPart          == NULL){
            LastPart            = Pez;
          }
        } else {
          LastPart              = NULL;
        }
        Pez++;
      }
      if(LastPart              != NULL ){
        int ISkip               = int(LastPart - TheStrBuffer);
        Truncate( ISkip );
      }
      return(*this);
    }   
    DCStr&        TrimRight       (CHAR ThisTarget){
      char TheTarget[2]         = { ThisTarget, 0 };
      return TrimRight(TheTarget);
    }   
    DCStr&        Trim            (CSTR ThisTarget = " "){
      return( TrimRight( ThisTarget ).TrimLeft( ThisTarget ) );
    }   
    DCStr&        Trim            (CHAR ThisTarget){
      char TheTarget[2]         = { ThisTarget, 0 };
      return(TrimRight(TheTarget).TrimLeft(TheTarget));
    }   
    DCStr         intValue        (int ThisValue, int Base = 10){
      char ThisBuffer[128];
      _itoa(ThisValue, ThisBuffer, Base);
      return SetString(ThisBuffer);
    }

    // I GUESS IT'S TOO MUCH TO ASK
      // DCStr         FormatValue   (MultiVal ThisValue, CSTR ThisFormat = "%d"){
      //   // LINK TO MS SUPPORTED FORMAT:
      //   // https://learn.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions?view=msvc-170

      //   char ThisBuffer[256];
      //   sprintf(ThisBuffer, ThisFormat, ThisValue);
      //   return SetString(ThisBuffer);
      // }
    // +

    DCStr         Copy            (CSTR ThisStr){
      return SetString(ThisStr);
    }
   // MAIN CAST
    operator      CSTR            () const throw(){
      return(TheStrBuffer);
    }   
    CSTR GetString                () const throw(){
      return(TheStrBuffer);
    }

    // When is the Copy Constructor Called?
      // In C++, a copy constructor may be called in the following cases: 

      // When an object of the class is returned by value.
      // When an object of the class is passed (to a function) by value as an argument.
      // When an object is constructed based on another object of the same class.
      // When the compiler generates a temporary object.
    DCStr(CDSTR& ThisStr){ // {TheStrBuffer=0xcccccccc <Error reading characters of string.> TheLen=-858993460 TheBffSize=-858993460 ...}
      const void* CheckStr      = (const void*)(ThisStr);
      if(CheckStr){
        SetString(ThisStr.TheStrBuffer);
      }
    }
   // Assignment operators
    DCStr&        operator=       (CDSTR& ThisStr){
      // KEPT IN COMMENT FOR REMEMBERING THAT SHIT!
      // THE RETURN IS IRRELEVANT...
      // WHAT MATTERS IS WHO'S MAKING THE COPY OF WHAT!
      // IN THIS CASE IT'S THE RECIEVER'S.

      // return DCStr(ThisStr.TheStrBuffer);
      // return( *this );
      return SetString(ThisStr);
    }  
    DCStr&        operator=       (CSTR ThisStr){
      return SetString(ThisStr);
    }  
    CHR           operator[]      (int Indx){
      return TheStrBuffer[Indx];
    }


   // OPERATORS
    friend DCStr  operator+       (CDSTR& Str1, CDSTR& Str2){
      return Concatenate( Str1, Str2 );
    }   
    friend DCStr  operator+       (CDSTR& Str1, CSTR Str2){
      return Concatenate( Str1, Str2 );
    }   
    friend DCStr  operator+       (CSTR Str1,   CDSTR& Str2){
      return Concatenate( Str1, Str2 );
    }   
    friend DCStr  operator+       (CDSTR& Str1, CHAR Chr2){
      char TheCharStr[2]        = { Chr2, 0 };
      return Concatenate(Str1,    TheCharStr);
    }   
    friend DCStr  operator+       (CHAR Chr1,   CDSTR& Str2){
      char TheCharStr[2]        = { Chr1, 0 };
      return Concatenate(TheCharStr, Str2);
    }   

    DCStr&        operator+=      (CDSTR ThisStr){
      return Append(ThisStr);
    }   
    DCStr&        operator+=      (CSTR  ThisStr){
      return Append(ThisStr);
    }   
    DCStr&        operator+=      (CHAR  ThisChar){
      return AppendChar(ThisChar);
    }


   // COMPARE
    friend bool   operator==      (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) == 0 );
    }   
    friend bool   operator==      (CDSTR& Str1,  CSTR Str2)    throw(){
      return(CompareString(Str1, Str2) == 0 );
    }   
    friend bool   operator==      (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) == 0 );
    }   

    friend bool   operator!=      (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) != 0 );
    }   
    friend bool   operator!=      (CDSTR& Str1,  CSTR Str2)    throw(){
      return(CompareString(Str1, Str2) != 0 );
    }   
    friend bool   operator!=      (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) != 0 );
    }   

    friend bool   operator>=      (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) >= 0 );
    }   
    friend bool   operator>=      (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2) >= 0 );
    }   
    friend bool   operator>=      (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) >= 0 );
    }   

    friend bool   operator<=      (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2) <= 0 );
    }   
    friend bool   operator<=      (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2) <= 0 );
    }   
    friend bool   operator<=      (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2) <= 0 );
    }   

    friend bool   operator>       (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2)  > 0 );
    }   
    friend bool   operator>       (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2)  > 0 );
    }   
    friend bool   operator>       (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2)  > 0 );
    }
    
    friend bool   operator<       (CDSTR& Str1,  CDSTR& Str2 ) throw(){
      return( CompareString(Str1, Str2)  < 0 );
    }   
    friend bool   operator<       (CDSTR& Str1,  CSTR Str2)    throw(){
      return( CompareString(Str1, Str2)  < 0 );
    }   
    friend bool   operator<       (CSTR Str1,    CDSTR& Str2)  throw(){
      return( CompareString(Str1, Str2)  < 0 );
    }
  }; 
// ----------- vsprintf that doesn't CRASH ---------
  int VStrFormat(CSTR ThisFormat, PSTR TheBuffer, int TheSize, va_list TheVList){
  int   ThisLen   = 0;
  char  Previous  = '\0';
  union PrintUnion {
    int     AsInt;
    float   AsFloat;
    char    AsChar;
    char*   AsString;
  } Printable;
  for(int Indx   = 0;         ThisFormat[Indx] != 0; ++Indx){
    char ThisChar           = ThisFormat[Indx];    
    if(ThisLen             >= TheSize - 1){
      TheBuffer[TheSize]    = 0;
      return TheSize;
    }
    if(Previous            == '%'){
      switch( ThisChar ) {   
      case 'd':{
        char NumBuffer[128];
        Printable.AsInt     = va_arg( TheVList, int );
        itoa(Printable.AsInt, NumBuffer, 10);
        int ThisNumLen      = strlen(NumBuffer);
        if(ThisNumLen       < (TheSize - ThisLen)){
          strncpy(&TheBuffer[ThisLen], NumBuffer, ThisNumLen);
          ThisLen          += ThisNumLen;
        }
      } break;
      case 's':{
        Printable.AsString  = va_arg( TheVList, char* );
        int ThisStrLen      = strlen(Printable.AsString);
        if(ThisStrLen       < (TheSize - ThisLen)){
          strncpy(&TheBuffer[ThisLen], Printable.AsString, ThisStrLen);
          ThisLen          += (ThisStrLen);
        }
      } break;
      case 'c':
        Printable.AsChar    = va_arg( TheVList, char );
        TheBuffer[ThisLen++]= Printable.AsChar;
        break;
      case '%':
        TheBuffer[ThisLen++]= '%';
        break;
      default:
        if(ThisLen          < (TheSize - 3)){
          TheBuffer[ThisLen++]= 'N';
          TheBuffer[ThisLen++]= 'A';
        }
        break;
      }
    } else 
    if(ThisChar            != '%'){
      TheBuffer[ThisLen++]  = ThisChar;
    }
    Previous                = ThisChar;
  }
  TheBuffer[ThisLen]        = 0;
  return ThisLen;
}

#endif // DCSTRING_H
