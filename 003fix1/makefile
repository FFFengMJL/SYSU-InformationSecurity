GCC := gcc 								# 编译器
HMAC-MD5 := ./hmac-md5.c 	# hmac-md5 语言源代码
MD5 := ./md5.c 						# md5 算法代码
MD5TEST := ./md5test.c 		# md5test 代码

ORIGINFILE := ./input.txt 	# 原始文本文件
KEYFILE := ./key.txt 				# 密钥储存文件
OUTPUTFILE := ./output.txt 	# 私钥储存文件

# hmac-md5 测试
htest: hmac-md5
	@./hmac-md5 ${ORIGINFILE} ${KEYFILE}

# 生成 hmac-md5 可执行文件
hmac-md5: ${HMAC-MD5} ${MD5}
	@${GCC} ${HMAC-MD5} -o $@

# md5 测试
mtest: md5
	@./md5 ${ORIGINFILE}

# 生成 md5test 文件
md5: ${MD5TEST} ${MD5}
	@${GCC} ${MD5TEST} -o $@

# 清空垃圾文件
clean: 
	@rm md5 || exit 0
	@rm hmac-md5  || exit 0