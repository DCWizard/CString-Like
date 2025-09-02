#include "stdafx.h"
#include "DCString.h"

int main(int argc, char* argv[]){
    
  DCStr ThisHeader = 
      "HTTP/1.1 101 Web Socket Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "WebSocket-Origin: \r\n"
      "WebSocket-Location: ws://:%d/\r\n"
      "Sec-WebSocket-Accept: \r\n"
      "WebService-ComAck: \r\n"
      "\r\n"
      ;

  int nPos = 0;
      #define RedefCast(A, B) reinterpret_cast< A >(B)

    // void* StrLine   = reinterpret_cast< void* >(&ThisHeader.Tokenize("\r\n", nPos));
    // void* StrLine   = RedefCast(void*,&ThisHeader.Tokenize("\r\n", nPos));
    // void* StrLine   = ReCast(void*)(&ThisHeader.Tokenize("\r\n", nPos));
  
  DCStr& StrLine    = (ThisHeader.Tokenize("\r\n", nPos));
    DCStr ThatDCStr = StrLine;
    // DCStr ThatDCStr;    ThatDCStr.Copy(StrLine);

    while(!(StrLine.IsEmpty())){
      printf("%s\n", StrLine);
      StrLine       = ThisHeader.Tokenize("\r\n", nPos);
    }
    DCStr Test      = StrLine;



  DCStr ThisName    = "Danni";
  DCStr ThisDCStr;
  // ThisDCStr.Format("Welcome back %s.\n", ThisName);
  // printf("This is the var %s", ThisDCStr);
 
  DCStr Th01DCStr   = ThisDCStr.Format("Welcome back %s.\n", ThisName);
  DCStr Th02DCStr   = Th01DCStr + "";

  Th02DCStr.Replace("Welcome back", "Bonjour");
  printf(Th02DCStr);

  Th02DCStr.Replace('o', '0');
  printf(Th02DCStr);


  Th02DCStr.Replace("B0nj0ur", "Welcome back");
  printf(Th02DCStr);


  Th02DCStr.Insert(12,'Q');
  printf(Th02DCStr);

  Th02DCStr.Insert(13,"TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST-TEST");
  printf(Th02DCStr);

  Th02DCStr.Remove('E');
  printf(Th02DCStr);

  Th02DCStr.Delete(12, 92);
  printf(Th02DCStr);

  Th02DCStr         = "      TEST Trimmed";
  Th02DCStr.TrimLeft();
  printf(Th02DCStr);
  
  Th02DCStr         = "      TEST Trimmed      ";
  Th02DCStr.TrimLeft(' ');
  printf(Th02DCStr);

  Th02DCStr         = "      TEST Trimmed      ";
  Th02DCStr.TrimRight();
  printf(Th02DCStr);

  Th02DCStr         = "      TEST Trimmed      ";
  Th02DCStr.TrimRight(' ');
  printf(Th02DCStr);

  Th02DCStr         = "      TEST Trimmed      ";
  Th02DCStr.Trim();
  printf(Th02DCStr);

  Th02DCStr         = "      TEST Trimmed      ";
  Th02DCStr.Trim(' ');
  printf(Th02DCStr);
  printf("|||\n");




  DCStr Th03DCStr   = Th02DCStr;
  DCStr Th04DCStr   = Th02DCStr.Mid(8, 4);
  printf("%s\n", Th04DCStr);

  DCStr Th05DCStr   = "";
        Th05DCStr  += 'A';

  if(Th05DCStr     == "A"){
    printf("Th05DCStr is equal. %s\n", Th05DCStr);  
  }
        Th05DCStr  += 'B';
  
  DCStr Th06DCStr   = Th05DCStr  += 'C';

        Th05DCStr  += " This str.";
        Th06DCStr   = Th05DCStr  + 'D';


  Th03DCStr         = "";
  Th03DCStr         = "New String.";
  if(Th03DCStr     != "New String2."){
    printf("New String is different. %s\n", Th03DCStr);  
  }

  Th03DCStr        += " And more.";

  DCStr ThisStr     = Th03DCStr;
  // CSTR ThisDCVar          = Th01DCStr;
  // const char* ThisStr           = &ThisDCVar[6];
  // char  ThatStr           = ThisDCVar[4];

  printf("This is the DC var: %s\n", ThisStr);



  return 0;
}
