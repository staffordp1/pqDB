/* pqDB.cpp */

#include "priv.hpp"
#include "getPass.hpp"

string *PASSWORD;
extern bool DEBUG;

/* defines global parameter PASSWORD used for orDB and pqDB*/
bool getPass()
{
  char *myPassWord;
  char *block;
  int    index, bIndex, passInx, numBlocks, fd, status;
  long buffSize;

  if(PASSWORD != NULL) delete PASSWORD;
  PASSWORD=0;

  myPassWord = (char *)malloc(32);

  /* Read Password File */
  fd = open(PASSWORD_FILE, O_RDONLY);
  if (fd == 0) {
        printf("Unable to open %s for input of encrypted myPassWord.\n", PASSWORD_FILE);
        return false;
  }

  status = read(fd, &numBlocks, sizeof(int));
  if (status == -1) {
        printf("Unable to read %s for input of encrypted myPassWord.\n", PASSWORD_FILE);
        close(fd);
        return false;
  }

  buffSize = numBlocks * DES_BLOCK_SIZE;
  block = (char *)malloc(buffSize);
  if (block == NULL) {
        printf("Out of memory.\n");
        return false;
  }

  status = read(fd, block, buffSize);
  if (status == -1) {
        printf("Unable to read %s for input of encrypted myPassWord.\n", PASSWORD_FILE);
        close(fd);
        return false;
  }
  close(fd);

  /* Set Decryption Key */
  setkey(key);

  /* Decrypt & Unload Encryption Block */
  memset(myPassWord, sizeof(myPassWord), 0);
  passInx = 0;

  for (bIndex = 0; bIndex < buffSize; bIndex += DES_BLOCK_SIZE)
  {

         encrypt(block + bIndex, 0);
         for (index = 0; index < DES_BLOCK_SIZE; index += BITS_IN_A_BYTE)
         {

             myPassWord[passInx++] = ((*(block + bIndex + index + 0) * 128) +
                                (*(block + bIndex + index + 1) * 64) +
                                (*(block + bIndex + index + 2) * 32) +
                                (*(block + bIndex + index + 3) * 16) +
                                (*(block + bIndex + index + 4) * 8) +
                                (*(block + bIndex + index + 5) * 4) +
                                (*(block + bIndex + index + 6) * 2) +
                                (*(block + bIndex + index + 7) * 1));
         }
  }

  PASSWORD = new string(myPassWord);
  return true;
}
