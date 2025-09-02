
#pragma once
#ifndef DCSTRING_H
#define DCSTRING_H

// Metadata
#define SHOW_Version "DCStr"
#define METHOD "ClearCode TM" // CREATED IN THE 90'S -> ONE GIANT STEP BETTER THAN CLEAN CODE.
#define THE_CopyRightNotice "`Copyright (c) 2020-2025 Denis Cote ThinknSpeak TM"

// Standard headers for Linux/portable C++
#include <cstring>    // strlen, strcpy, strncpy, strcmp, strcat, strchr, strrchr, strstr, memset, memmove, memcpy
#include <strings.h>  // strcasecmp
#include <cstdio>     // vsnprintf
#include <cstdlib>    // malloc, free
#include <cstdarg>    // va_list, va_start, va_end
#include <mutex>      // timed_mutex
#include <chrono>     // chrono::milliseconds
#include <limits>     // numeric_limits

using namespace std;

// Types and aliases to preserve original API
typedef char  CHR;
typedef char  CHAR;
typedef char* PSTR;
typedef const char* CSTR;
typedef unsigned char  BYTE;
typedef unsigned char* PBYTE;
typedef const unsigned char* CBYTE;

typedef unsigned long DWORD; // approximate Windows DWORD
typedef bool BOOL;

// Utility macros from original code
#define ReCast(A) reinterpret_cast< A >
#define null NULL

// Provide Windows-like helpers on Linux
#define ZeroMemory(ptr, sz) memset((ptr), 0, (sz))
#define strcmpi(a, b) ::strcasecmp((a), (b))

// Threading compatibility layer
// Emulate HANDLE with a timed mutex pointer
using HANDLE = timed_mutex*;
static constexpr DWORD INFINITE = numeric_limits<DWORD>::max();

// CLEAN ACCESS TO VAR TYPE
#define DSTR  DCStr
#define CDSTR const DCStr

// ---------------- class DCStr --------------------
class DCStr {
 public:
  PSTR TheStrBuffer = NULL;
  int  TheLen       = 0;
  int  TheBffSize   = 0;

  // Keep original constants/macros
  #define DCStrAppendZone 32
  #define DCStrEndingZone 8
  #define DCStrBufferZone DCStrAppendZone + DCStrEndingZone

  // Mutex (portable implementation)
  HANDLE StrMutex         = NULL;

  HANDLE InitStrMutex       (){
    if(StrMutex          == NULL){
      StrMutex            = new timed_mutex();
    }
    return StrMutex;
  }
  void DelStrMutex          (){
    if(StrMutex){
      delete StrMutex;
      StrMutex            = NULL;
    }
  }
  BOOL AcquireStrMutex      (DWORD ThisTimeout = INFINITE){
    if(!StrMutex){ 
      return false;
    }
    if(ThisTimeout       == INFINITE){
      StrMutex->lock();
      return true;
    } else {
      return StrMutex->try_lock_for(chrono::milliseconds(ThisTimeout));
    }
  }
  BOOL ReleaseStrMutex      (){
    if(StrMutex){
      StrMutex->unlock();
      return true;
    }
    return false;
  }
  void InitDCtring          (){
    InitStrMutex();
  }
  ~DCStr                    () noexcept {
    if(TheStrBuffer){
      free(TheStrBuffer);
      TheStrBuffer        = NULL;
      TheLen              = 0;
      TheBffSize          = 0;
    }
    DelStrMutex();
  }
  DCStr                     () noexcept {
    InitDCtring();
    TheBffSize            = DCStrAppendZone + DCStrEndingZone;
    TheStrBuffer          = (PSTR)malloc(TheBffSize);
    ZeroMemory(TheStrBuffer, TheBffSize);
  }
  DCStr(CSTR ThisStr){
    InitDCtring();
    SetString(ThisStr);
  }
  void EnterCriticalArea    (){
    AcquireStrMutex();
  }
  void LeaveCriticalArea    (){
    ReleaseStrMutex();
  }


  BOOL            Preallocate   (int ThisSize){
    if(ThisSize              <= TheLen){
      return true;
    }
    if(ThisSize              <= (TheLen + DCStrAppendZone)){
      return true;
    }
    EnterCriticalArea();
    TheBffSize                = ThisSize + DCStrBufferZone;
    PSTR TheTmpBuffer         = (PSTR)malloc(TheBffSize);
    ZeroMemory(TheTmpBuffer, TheBffSize);
    if(TheStrBuffer){
      strcpy(TheTmpBuffer, TheStrBuffer);
      free(TheStrBuffer);
    }
    TheStrBuffer              = TheTmpBuffer;
    LeaveCriticalArea();
    return true;
  }
  DCStr&          SetString     (CSTR ThisStr){
    EnterCriticalArea();
    if((TheLen                > 0) 
    && (TheStrBuffer         != NULL)
    ){
      free(TheStrBuffer);
      TheStrBuffer            = NULL;
    }
    if(!ThisStr){ 
      ThisStr                 = "";
    }
    TheLen                    = strlen(ThisStr);
    TheBffSize                = TheLen + DCStrBufferZone;
    TheStrBuffer              = (PSTR)malloc(TheBffSize);
    ZeroMemory(TheStrBuffer,    TheBffSize);
    strcpy(TheStrBuffer,        ThisStr);
    LeaveCriticalArea();
    return(*this);
  }
  DCStr&          Append        (CSTR ThisStr) noexcept {
    if(!ThisStr){ 
      ThisStr                 = "";
    }
    if(TheLen                <= 0){
      return SetString(ThisStr);
    }
    EnterCriticalArea();
    int TheTmpLen             = strlen(ThisStr);
    int TheTltLen             = TheLen + TheTmpLen;
    if(TheTmpLen              > 0){
      Preallocate(TheTltLen);
      strcpy(&TheStrBuffer[TheLen], ThisStr);
      TheLen                  = strlen(TheStrBuffer);
    }
    LeaveCriticalArea();
    return(*this);
  }
  DCStr&          AppendChar    (CHAR ThisChar){
    char buf[2]               = { ThisChar, 0 };
    return Append(buf);
  }
  int             Insert        (int ThisStart, CSTR ThisInsert){
    if(!ThisInsert){ 
      ThisInsert              = "";
    }
    if(ThisStart              < 0){
      ThisStart               = 0;
    } else 
    if(ThisStart              > TheLen){
      ThisStart               = TheLen;
    }
    int InsertLength          = strlen(ThisInsert);
    int TheNewLen             = TheLen;
    if(InsertLength           > 0){
      TheNewLen              += InsertLength;
      EnterCriticalArea();
      Preallocate(TheNewLen);

      int  StartWhere         = ThisStart;
      int  FinishWhere        = StartWhere + InsertLength;
      int  TheRestLen         = TheLen - (StartWhere);
      CSTR TheRestStr         = &TheStrBuffer[StartWhere];

      memmove(&TheStrBuffer[FinishWhere], TheRestStr, strlenTheRestLen);
      memcpy( &TheStrBuffer[StartWhere],  ThisInsert, strlenInsertLength);
      TheLen = strlen(TheStrBuffer);
      LeaveCriticalArea();
    }
    return(TheNewLen);
  }
  int             Insert        (int ThisStart, CHAR ThisChar){
    char buf[2]               = { ThisChar, 0 };
    return Insert(ThisStart, buf);
  }
  DCStr&          Format        (CSTR ThisFormat, ...){
    char ThisBuffer[4096 + DCStrBufferZone];
    if(!(ThisFormat)){ 
      ThisFormat              = "";
    }
    va_list   argList;
    va_start( argList, ThisFormat);
    vsnprintf(ThisBuffer, sizeof(ThisBuffer), ThisFormat, argList);
    va_end(   argList);
    return SetString(ThisBuffer);
  }
  static DCStr    Concatenate   (CSTR ThisStr, CSTR ThatStr){
    if(!(ThisStr)){ 
      ThisStr                 = "";
    }
    if(!(ThatStr)){ 
      ThatStr                 = "";
    }
    int TheTm1Len             = strlen(ThisStr);
    int TheTm2Len             = strlen(ThatStr);
    int TheTltLen             = TheTm1Len + TheTm2Len;
    if(TheTm2Len              > 0){
      PSTR TheTmpBuffer       = (PSTR)malloc(TheTltLen + DCStrBufferZone);
      ZeroMemory(TheTmpBuffer,  TheTltLen + DCStrBufferZone);
      strcpy(TheTmpBuffer,      ThisStr);
      strcpy(&TheTmpBuffer[TheTm1Len], ThatStr);
      DCStr ThisRetStr        = TheTmpBuffer;
      free(TheTmpBuffer);
      return ThisRetStr;
    }
    return DCStr(ThisStr);
  }
  static int      CompareString (CSTR Str1, CSTR Str2) noexcept {
    if(!(Str1)){ 
      Str1                    = "";
    }
    if(!(Str2)){ 
      Str2                    = "";
    }
    return strcmp(Str1, Str2);
  }
  static int      CompareNoCase (CSTR Str1, CSTR Str2) noexcept {
    if(!(Str1)){ 
      Str1                    = "";
    }
    if(!(Str2)){ 
      Str2                    = "";
    }
    return ::strcasecmp(Str1, Str2);
  }
  int             Compare       (CSTR ThisStr){
    return CompareString(TheStrBuffer, ThisStr);
  }
  CSTR            GetBuffer     (){
    return TheStrBuffer;
  }
  int             GetBufferLen  () const noexcept {  // MEASURE UP TO DCStrAppendZone - DCStrEndingZone
    return(TheBffSize - DCStrEndingZone);
  }
  int             GetLength     (){
    return TheLen;
  }
  bool            IsEmpty       (){
    return(TheLen           == 0);
  }
  void            Empty         (){
    if(TheLen                == 0){
      return;
    }
    ZeroMemory(TheStrBuffer, TheBffSize);
    TheLen = 0;
  }
  void            Truncate      (int ThisNewLen){
    if(TheLen                 > ThisNewLen){
      TheLen                  = ThisNewLen;
      int ThisGap             = TheBffSize - TheLen;
      ZeroMemory(&TheStrBuffer[TheLen], ThisGap);
    }
  }
  char            GetAt         (int Indx){
    if((Indx                  < 0) 
    || (Indx                 >= TheLen)
    ){
      return 0;
    }
    return TheStrBuffer[Indx];
  }
  BOOL            SetAt         (int Indx, CHAR ThisChar){
    if((Indx                  < 0) 
    || (Indx                 >= TheLen)
    ){
      return false;
    }
    TheStrBuffer[Indx]        = ThisChar;
    // keep length as-is; ensure null terminator still in place
    return true;
  }
  // Note: default length sentinel changed to -1 to avoid ambiguity with 0
  DCStr           Make          (CSTR ThisStr, int ThisLen = -1){
    if(!(ThisStr)){ 
      ThisStr                 = "";
    }
    if(ThisLen                < 0){
      ThisLen                 = strlen(ThisStr);
    }
    PSTR ThisTempStr          = (PSTR)malloc(strlenThisLen + DCStrBufferZone);
    ZeroMemory(ThisTempStr,     strlenThisLen + DCStrBufferZone);
    strncpy(ThisTempStr,        ThisStr, strlenThisLen);
    ThisTempStr[ThisLen]      = 0;
    DCStr ThisOutput          = ThisTempStr;
    free(ThisTempStr);
    return ThisOutput;
  }
  DCStr           Left          (int Indx){
    if((Indx                  < 0)){
      return DCStr("");
    }
    if((Indx                 >= TheLen)){
      return DCStr(TheStrBuffer ? TheStrBuffer : "");
    }
    return Make(TheStrBuffer ? TheStrBuffer : "", Indx);
  }
  DCStr           Right         (int Indx){
    if((Indx                  < 0)){
      return DCStr("");
    }
    if((Indx                 >= TheLen)){
      return DCStr(TheStrBuffer ? TheStrBuffer : "");
    }
    int TheStart              = TheLen - Indx;
    return Make(&TheStrBuffer[TheStart]);
  }
  DCStr           Mid           (int ThisStart, int ThisLen = -1){
    if(ThisLen                < 0){
      return Right(TheLen - ThisStart);
    }
    if((ThisStart             < 0)){
      ThisStart               = 0;
    } else if((ThisStart     >= TheLen)){
      return DCStr("");
    }
    return Make(&TheStrBuffer[ThisStart], ThisLen);
  }
  int             Find          (CSTR ThisMatch, int ThisStart = 0){
    if(!(ThisMatch)){ 
      ThisMatch               = "";
    }
    if((ThisStart             < 0)){
      ThisStart               = 0;
    } else 
    if((ThisStart            >= TheLen)){
      return -1;
    }
    CSTR ThisRet              = strstr(&TheStrBuffer[ThisStart], ThisMatch);
    return(ThisRet == NULL) ? -1 : int(ThisRet - TheStrBuffer);
  }
  int             Find          (CHAR ThisMatch, int ThisStart = 0){
    if((ThisStart             < 0)){
      ThisStart               = 0;
    } else 
    if((ThisStart            >= TheLen)){
      return -1;
    }
    CSTR ThisRet = strchr(&TheStrBuffer[ThisStart], ThisMatch);
    return(ThisRet == NULL) ? -1 : int(ThisRet - TheStrBuffer);
  }
  int             ReverseFind   (CHAR ThisMatch, int ThisStart = 0){
    if((ThisStart             < 0)){
      ThisStart               = 0;
    } else 
    if((ThisStart            >= TheLen)){
      return -1;
    }
    CSTR ThisRet              = strrchr(&TheStrBuffer[ThisStart], ThisMatch);
    return(ThisRet == NULL) ? -1 : int(ThisRet - TheStrBuffer);
  }
  int             Replace       (CSTR ThisMatch, CSTR ThisReplace){
    if(!(ThisMatch)){ 
      ThisMatch               = "";
    }
    if(!(ThisReplace)){ 
      ThisReplace             = "";
    }
    CSTR ThisFound;
    int  ThisStart            = 0;
    int  FoundCount           = 0;
    int  MatchLen             = strlen(ThisMatch);
    int  ReplaceLen           = strlen(ThisReplace);
    int  ReplaceDiff          = ReplaceLen - MatchLen;
    if(MatchLen               <= 0) return 0;

    EnterCriticalArea();
    do {
      ThisFound               = strstr(&TheStrBuffer[ThisStart], ThisMatch);
      if(ThisFound){
        FoundCount++;
        int  StartWhere       = int(ThisFound - TheStrBuffer);
        int  FinishWhere      = StartWhere + ReplaceLen;
        int  TheRestLen       = TheLen - (StartWhere + MatchLen);
        CSTR TheRestStr       = &TheStrBuffer[StartWhere + MatchLen];

        if(ReplaceDiff        > 0){ // need more buffer
          int TheNewLen       = (TheLen + ReplaceDiff);
          Preallocate(TheNewLen);
          // move rest to make room
          memmove(&TheStrBuffer[FinishWhere], TheRestStr, strlenTheRestLen);
          memcpy( &TheStrBuffer[StartWhere],  ThisReplace, strlenReplaceLen);
          TheLen              = strlen(TheStrBuffer);
        } else {
          // replace then close the gap if any
          memcpy(&TheStrBuffer[StartWhere],   ThisReplace, strlenReplaceLen);
          strcpy(&TheStrBuffer[FinishWhere],  TheRestStr);
          TheLen              = strlen(TheStrBuffer);
        }
        ThisStart             = FinishWhere;
      }
    } while(ThisFound);
    LeaveCriticalArea();
    return FoundCount;
  }
  int             Replace       (CHAR ThisMatch, CHAR ThisReplace){
    char TheMatch[2]          = { ThisMatch, 0 };
    char TheReplace[2]        = { ThisReplace, 0 };
    return Replace(TheMatch,    TheReplace);
  }
  DCStr           Tokenize      (CSTR ThisToken, int& ThisStart){
    if(ThisStart              < 0){
      ThisStart               = 0;
    }
    if((ThisToken            == NULL) 
    || (*ThisToken           == (CHAR)0)
    ){
      if(ThisStart            < TheLen){
        return DCStr(TheStrBuffer + ThisStart);
      }
    } else {
      CSTR StartStr           = TheStrBuffer + ThisStart;
      CSTR EndStr             = TheStrBuffer + TheLen;
      if(StartStr             < EndStr){
        int Including         = (int)strspn(StartStr, ThisToken);

        if(EndStr            >= (StartStr    + Including)){
          StartStr           += Including;
          int Excluding       = (int)strcspn(StartStr, ThisToken);
          int ThisFrom        = ThisStart + Including;
          int UntilTo         = Excluding;
          DCStr ThisRet       = Mid(ThisFrom, UntilTo);
          ThisStart           = ThisFrom + UntilTo + 1;

          return(ThisRet);
        }
      }
    }
    // return empty string, done tokenizing
    ThisStart                 = -1;
    DCStr ThisRet             = Make("");
    return(ThisRet);
  }
  int             Delete        (int Index, int ThisCount = 1){
    if(Index                  < 0){
      Index                   = 0;
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
  int             Remove        (CHAR RemoveChar){
    PSTR CurSrc               = TheStrBuffer;
    PSTR CurDst               = TheStrBuffer;
    PSTR EndStr               = TheStrBuffer + TheLen;

    while(CurSrc              < EndStr){
      PSTR NewSrc             = CurSrc + 1;
      if(*CurSrc             != RemoveChar){
        int  NewSrcGap        = int(NewSrc - CurSrc);
        PSTR NewDst           = CurDst + NewSrcGap;
        for (int Indx = 0; (CurDst != NewDst) && (Indx < NewSrcGap); Indx++){
          *CurDst             = *CurSrc;
          CurSrc++;
          CurDst++;
        }
      }
      CurSrc                  = NewSrc;
    }
    *CurDst                   = 0;
    TheLen                    = strlen(TheStrBuffer);
    return(TheLen);
  }
  DCStr&          TrimLeft      (CSTR ThisTarget = " "){
    if((ThisTarget           == NULL) 
    || (*ThisTarget          == 0)
    ){
      return(*this);
    }
    CSTR Pez                  = TheStrBuffer; // Pez dispenser.
    while((*Pez              != 0) 
    &&    (NULL              != strchr(ThisTarget, *Pez))
    ){
      Pez++;
    }
    if(Pez                   != TheStrBuffer){
      int ISkip               = int(Pez - TheStrBuffer);
      int TheNewLen           = TheLen  - ISkip;
      memmove(TheStrBuffer,     Pez, (TheNewLen + 1));
      TheLen                  = strlen(TheStrBuffer);
    }
    return(*this);
  }
  DCStr&          TrimLeft      (CHAR ThisTarget){
    char TheTarget[2]         = { ThisTarget, 0 };
    return TrimLeft(TheTarget);
  }
  DCStr&          TrimRight     (CSTR ThisTarget = " "){
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
    if(LastPart              != NULL){
      int ISkip               = int(LastPart - TheStrBuffer);
      Truncate(ISkip);
    }
    return(*this);
  }
  DCStr&          TrimRight     (CHAR ThisTarget){
    char TheTarget[2]         = { ThisTarget, 0 };
    return TrimRight(TheTarget);
  }
  DCStr&          Trim          (CSTR ThisTarget = " "){
    return(TrimRight(ThisTarget).TrimLeft(ThisTarget));
  }
  DCStr&          Trim          (CHAR ThisTarget){
    char TheTarget[2]         = { ThisTarget, 0 };
    return(TrimRight(TheTarget).TrimLeft(TheTarget));
  }
  DCStr           Copy          (CSTR ThisStr){
    return SetString(ThisStr);
  }
 // MAIN CAST
  operator        CSTR          () const noexcept {
    return(TheStrBuffer);
  }
 // Copy constructor
  DCStr(CDSTR& ThisStr){
    SetString(ThisStr.TheStrBuffer);
  }
 // Assignment operators
  DCStr&          operator=     (CDSTR& ThisStr){
    return SetString(ThisStr);
  }
  DCStr&          operator=     (CSTR ThisStr){
    return SetString(ThisStr);
  }
  CHR             operator[]    (int Indx){
    return TheStrBuffer[Indx];
  }

 // OPERATORS
  friend DCStr    operator+     (CDSTR& Str1, CDSTR& Str2){
    return Concatenate(Str1, Str2);
  }
  friend DCStr    operator+     (CDSTR& Str1, CSTR Str2){
    return Concatenate(Str1, Str2);
  }
  friend DCStr    operator+     (CSTR Str1, CDSTR& Str2){
    return Concatenate(Str1, Str2);
  }
  friend DCStr    operator+     (CDSTR& Str1, CHAR Chr2){
    char TheCharStr[2]        = { Chr2, 0 };
    return Concatenate(Str1, TheCharStr);
  }
  friend DCStr    operator+     (CHAR Chr1, CDSTR& Str2){
    char TheCharStr[2]        = { Chr1, 0 };
    return Concatenate(TheCharStr, Str2);
  }

  DCStr&          operator+=    (CDSTR ThisStr){
    return Append(ThisStr);
  }
  DCStr&          operator+=    (CSTR ThisStr){
    return Append(ThisStr);
  }
  DCStr&          operator+=    (CHAR ThisChar){
    return AppendChar(ThisChar);
  }

 // COMPARE
  friend bool     operator==    (CDSTR& Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) == 0);
  }
  friend bool     operator==    (CDSTR& Str1, CSTR Str2) noexcept {
    return(CompareString(Str1, Str2) == 0);
  }
  friend bool     operator==    (CSTR Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) == 0);
  }

  friend bool     operator!=    (CDSTR& Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) != 0);
  }
  friend bool     operator!=    (CDSTR& Str1, CSTR Str2) noexcept {
    return(CompareString(Str1, Str2) != 0);
  }
  friend bool     operator!=    (CSTR Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) != 0);
  }

  friend bool     operator>=    (CDSTR& Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) >= 0);
  }
  friend bool     operator>=    (CDSTR& Str1, CSTR Str2) noexcept {
    return(CompareString(Str1, Str2) >= 0);
  }
  friend bool     operator>=    (CSTR Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) >= 0);
  }

  friend bool     operator<=    (CDSTR& Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) <= 0);
  }
  friend bool     operator<=    (CDSTR& Str1, CSTR Str2) noexcept {
    return(CompareString(Str1, Str2) <= 0);
  }
  friend bool     operator<=    (CSTR Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) <= 0);
  }

  friend bool     operator>     (CDSTR& Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) > 0);
  }
  friend bool     operator>     (CDSTR& Str1, CSTR Str2) noexcept {
    return(CompareString(Str1, Str2) > 0);
  }
  friend bool     operator>     (CSTR Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) > 0);
  }

  friend bool     operator<     (CDSTR& Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) < 0);
  }
  friend bool     operator<     (CDSTR& Str1, CSTR Str2) noexcept {
    return(CompareString(Str1, Str2) < 0);
  }
  friend bool     operator<     (CSTR Str1, CDSTR& Str2) noexcept {
    return(CompareString(Str1, Str2) < 0);
  }
};

#endif // DCSTRING_H
