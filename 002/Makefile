GCC := gcc # 编译器
GMP := gmp # 连接库
SOURCE := ./rsa.c # C语言源代码

ORIGINFILE := ./in.txt # 原始文本文件
ENCRYPTEDFILE := ./encrypted.txt # 加密后的文件
DECRYPTEDFILE := ./decrypted.txt # 解密后的文件
PUBLICKEYFILE := ./publicKey.txt # 公钥储存文件
PRIVATEKEYFILE := ./privateKey.txt # 私钥储存文件

ENC := enc # 加密
DEC := dec # 解密
TEST := test # 测试

# 执行文件
a.out: ${SOURCE}
	@${GCC} ${SOURCE} -o $@ -l${GMP}

# 加密
enc: a.out
	@./a.out ${ENC} ${PUBLICKEYFILE} ${PRIVATEKEYFILE} ${ORIGINFILE} ${ENCRYPTEDFILE}

# 解密
dec: a.out 
	@./a.out ${DEC} ${PRIVATEKEYFILE} ${ENCRYPTEDFILE} ${DECRYPTEDFILE}

# 一个完整的流程
test: a.out
	@./a.out ${TEST} ${PUBLICKEYFILE} ${PRIVATEKEYFILE} ${ORIGINFILE} ${ENCRYPTEDFILE} ${DECRYPTEDFILE}