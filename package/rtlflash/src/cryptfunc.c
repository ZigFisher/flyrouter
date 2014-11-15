
void simplecrypt(char * buf, unsigned int len)
{
   //the key is somewhat long, has a decent (though not great) range of values,
   //isn't very easy to figure out (without poking into executable),
   //and changes each time it compiles--it's not a bad key for its purpose
   static const char key[]  = {"'a}z'#'x99'xb8'r'b'xa3&e!'>C~'x83"};
   static const int  keylen = sizeof(key)/sizeof(key[0]) - 1;

   unsigned int i;

   if(buf)
   {
      for(i = 0; i < len; i++)
         buf[i] ^= ~(char)((int)key[i % keylen] + 0xa3);
   }
}
