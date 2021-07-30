KEY = a1b2c3d4e5f6f7e8 # 密钥，请务必保证是64位
IN := ./in.txt # 输入的 txt 文件名

# openssl 相关，主要用于验证
SENC := ./senc.txt 			# openssl 加密输出的文件名
SDEC := ./sdec.txt 			# openssl 解密输出的文件名
ENCMODE := enc -e -des-ecb 	# 加密模式
DECMODE := enc -d -des-ecb 	# 解密模式

# C 代码相关
GCC := gcc 						# 编译器
INC := ./des.c 				# 源代码
OUTC := ./des 				# 编译出的程序
CENC := ./cenc.txt # 加密输出的文件名
CDEC := ./cdec.txt # 解密输出的文件名

# C 代码进行加密操作
enc:
	@${GCC} ${INC} -o ${OUTC}
	@${OUTC} enc ${KEY} ${IN} > ${CENC}
	@xxd ${CENC}

# C 代码进行解密操作
dec:
	@${GCC} ${INC} -o ${OUTC}
	@${OUTC} dec ${KEY} ${CENC} > ${CDEC}
	@xxd ${CDEC}

# 使用 openssl 进行加密操作
senc:
	@openssl ${ENCMODE} -K ${KEY} -in ${IN} -out ${SENC} 
	@xxd ${SENC}

# 使用 openssl 进行解密操作
sdec:
	@openssl ${DECMODE} -K ${KEY} -in ${SENC} -out ${SDEC} 
	@xxd ${SDEC}

# 比较 C 代码和 openssl 加密结果
enc-diff:
	@diff -y ${CENC} ${SENC} || exit 0
	@echo ''

# 比较 C 代码和 openssl 解密结果
dec-diff:
	@diff -y ${CDEC} ${SDEC} || exit 0
	@echo ''

# 清除
clean:
	@rm ${OUTC} || exit 0