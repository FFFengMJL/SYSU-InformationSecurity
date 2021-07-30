#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <gmp.h>

#define pqsize 1024

struct private_key
{
  mpz_t N;
  mpz_t e;
  mpz_t d;
  mpz_t p;
  mpz_t q;
};

struct public_key
{
  mpz_t N;
  mpz_t e;
};

void generate_keys(struct public_key *pubk, struct private_key *prik)
{
  printf("generateKey\n");
  //设置随机数种子
  gmp_randstate_t seed;
  gmp_randinit_default(seed);
  gmp_randseed_ui(seed, time(NULL));
  //调用gmp库函数生成大素数p和q
  mpz_urandomb(prik->p, seed, pqsize);
  mpz_nextprime(prik->p, prik->p);
  mpz_urandomb(prik->q, seed, pqsize);
  mpz_nextprime(prik->q, prik->q);
  //计算N=p*q
  mpz_mul(prik->N, prik->p, prik->q);
  //计算(p-1)*(q-1)
  mpz_t temp_p, temp_q, temp;
  mpz_init_set(temp_p, prik->p);
  mpz_init_set(temp_q, prik->q);
  mpz_init(temp);
  mpz_sub_ui(temp_p, temp_p, 1);
  mpz_sub_ui(temp_q, temp_q, 1);
  mpz_mul(temp, temp_p, temp_q);
  //设置e=65537
  mpz_set_ui(prik->e, 65537);
  //计算d
  mpz_invert(prik->d, prik->e, temp);
  //公钥赋值
  mpz_set(pubk->e, prik->e);
  mpz_set(pubk->N, prik->N);
}

int oct2int(char c)
{
  printf("oct2int\n");
  int result = 0, i;
  for (i = 0; i < 8; i++)
    result += ((c >> (7 - i)) & 1) * pow(2, 7 - i);

  return result;
}

char int2oct(int num)
{
  printf("int2oct\n");
  char result = 0, temp = 0;
  int i, j;
  for (i = 0; i < 8 && num != 0; i++)
  {
    temp = (temp << 1) + num % 2;
    num /= 2;
  }
  for (j = 0; j < i; j++)
    result = (result << 1) + ((temp >> j) & 1);

  return result;
}

void encoding(char *M, struct public_key key, char *EM, int num)
{
  printf("encoding\n");
  int k = mpz_sizeinbase(key.N, 2);
  k = k / 8 + k % 8 ? 1 : 0;
  int i, len = k - num - 3, temp;
  if (len < 8)
  {
    printf("Length of PS too short.\n");
    exit(-1);
  }

  EM[0] = 0x00;
  EM[1] = 0x02;
  srand((unsigned)time(NULL));
  for (i = 2; i < len + 2; i++)
    EM[i] = rand() % 254 + 1;
  EM[i] = 0x00;
  for (i = len + 3; i < k; i++)
    EM[i] = M[i - len - 3];
  EM[k] = '\0';
}

void encryption(char *M, struct public_key key, char *I2OSP)
{
  printf("encryption\n");
  int i, k = mpz_sizeinbase(key.N, 2);
  k = k / 8 + k % 8 ? 1 : 0;
  mpz_t OS2IP, RSAEP, temp;
  mpz_init(OS2IP);
  mpz_init(RSAEP);
  mpz_init_set(temp, key.N);
  mpz_sub_ui(temp, temp, 1);

  for (i = 0; i < k; i++)
    mpz_add_ui(OS2IP, OS2IP, (oct2int(M[i]) * pow(256, k - 1 - i)));

  if ((mpz_cmp_ui(OS2IP, 0) < 0) || (mpz_cmp(OS2IP, temp) > 0))
  {
    printf("Message representative out of range.\n");
    exit(-1);
  }
  mpz_powm(RSAEP, OS2IP, key.e, key.N);

  if (mpz_cmp_ui(RSAEP, pow(256, k)) < 0)
  {
    printf("Integer too large.\n");
    exit(-1);
  }
  for (i = 0; i < k; i++)
  {
    mpz_cdiv_r_ui(temp, RSAEP, 256);
    mpz_cdiv_q_ui(RSAEP, RSAEP, 256);
    I2OSP[k - 1 - i] = int2oct(mpz_get_si(temp));
  }
  I2OSP[k] = '\0';

  printf("EM : %s\n", I2OSP);
}

void decryption(char *C, struct private_key key, char *I2OSP)
{
  printf("decryption\n");
  int i;
  mpz_t OS2IP, RSADP, temp;
  mpz_init(OS2IP);
  mpz_init(RSADP);
  mpz_init_set(temp, key.N);
  mpz_sub_ui(temp, temp, 1);

  for (i = 0; i < strlen(C); i++)
    mpz_add_ui(OS2IP, OS2IP, (oct2int(C[i]) * pow(256, strlen(C) - 1 - i)));

  if ((mpz_cmp_ui(OS2IP, 0) < 0) || (mpz_cmp(OS2IP, temp) > 0))
  {
    printf("Ciphertext representative out of range.\n");
    exit(-1);
  }
  mpz_powm(RSADP, OS2IP, key.d, key.N);

  for (i = 0; i < strlen(C); i++)
  {
    mpz_cdiv_r_ui(temp, RSADP, 256);
    mpz_cdiv_q_ui(RSADP, RSADP, 256);
    I2OSP[strlen(C) - 1 - i] = int2oct(mpz_get_si(temp));
  }
  I2OSP[strlen(C)] = '\0';
}

void decoding(char *C, struct private_key key, char *M, int *num)
{
  printf("decoding\n");

  int i, j;

  if (C[0] != 0x00 || C[1] != 0x02)
  {
    printf("Decryption error.\n");
    exit(-1);
  }
  for (i = 2; i < strlen(C); i++)
    if (C[i] == 0x00)
      break;
  if (i - 2 < 8 || i == strlen(C))
  {
    printf("Decryption error.\n");
    exit(-1);
  }
  for (j = i + 1; j < strlen(C); j++)
  {
    M[j - i - 1] = C[j];
  }
  M[strlen(C)] = '\0';
  *num = strlen(C) - i - 1;
}

void rsa_encrypt(char *M, struct public_key key, char *C, int num)
{
  printf("rsa_encrypt\n");

  int k = mpz_sizeinbase(key.N, 2) / 8 + mpz_sizeinbase(key.N, 2) % 8 ? 1 : 0;
  if (num > k - 11)
  {
    printf("Message too long.\n");
    exit(-1);
  }
  char *EM = (char *)malloc(k + 1);
  encoding(M, key, EM, num);
  printf("Encrypt EM : %s\n", EM);
  encryption(EM, key, C);
}

void rsa_decrypt(char *C, struct private_key key, char *M, int *num)
{
  printf("rsa_decrypt\n");
  int k = mpz_sizeinbase(key.N, 2) / 8 + mpz_sizeinbase(key.N, 2) % 8 ? 1 : 0;
  if (*num != k || k < 11)
  {
    printf("Decryption error.\n");
    exit(-1);
  }
  char *I2OSP = (char *)malloc(k + 1);
  decryption(C, key, I2OSP);
  printf("Decrypt I2OSP : %s\n", I2OSP);
  decoding(I2OSP, key, M, num);
}

void rsa(char infile[])
{
  printf("rsa");
  int i, num;

  unsigned long k = 0;
  FILE *in = NULL, *out1 = NULL, *out2 = NULL;

  struct public_key pubk;
  struct private_key prik;
  mpz_init(pubk.e);
  mpz_init(pubk.N);
  mpz_init(prik.N);
  mpz_init(prik.e);
  mpz_init(prik.d);
  mpz_init(prik.p);
  mpz_init(prik.q);

  in = fopen(infile, "r");
  out1 = fopen("en.txt", "w");
  out2 = fopen("de.txt", "w");

  generate_keys(&pubk, &prik);
  gmp_printf("public key : \ne : %ZX\nN : %ZX\n", prik.e, pubk.N);
  gmp_printf("private key : \nd : %ZX\nN: %ZX\n", prik.d, prik.N);
  printf("start to get k\n");
  // mpz_t tmp;
  // mpz_init_set(tmp, prik.N);
  k = mpz_sizeinbase(prik.N, 2);
  k = k / 8 + (k % 8 ? 1 : 0);
  printf("k = %ld", k);
  char *Data = (char *)malloc(k - 10);
  num = fread(Data, sizeof(char), k, in);

  printf("fread first\n");
  char *en = (char *)malloc(k + 1);
  rsa_encrypt(Data, pubk, en, num);
  for (i = 0; i < k; i++)
    fputc(en[i], out1);
  fclose(out1);

  out1 = fopen("en.txt", "r");
  char *de = (char *)malloc(k + 1);
  num = fread(Data, sizeof(char), k, out1);
  rsa_decrypt(Data, prik, de, &num);
  for (i = 0; i < num; i++)
    fputc(de[i], out2);

  fclose(in);
  fclose(out1);
  fclose(out2);
}

int main()
{
  char infile[1000];

  printf("Please enter the path of in file : ");
  scanf("%s", infile);

  rsa(infile);

  return 0;
}