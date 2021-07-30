# 编译器
GCC := gcc
# 源代码文件
SRC := ./src
# client 代码
CLIENT := client.c
# TGS 代码
TGS := TGS.c
# SS 代码
SS := SS.c
# AS 代码
AS := AS.c
# des 加密
DES := des.c
# 主密钥文件
KEY_CLIENT := ./keyClient

all: client AS SS TGS

client: ${SRC}/${CLIENT} ${SRC}/${DES}
	@${GCC} $< -o $@

TGS: ${SRC}/${TGS} ${SRC}/${DES}
	@${GCC} $< -o $@

AS: ${SRC}/${AS} ${SRC}/${DES}
	@${GCC} $< -o $@

SS: ${SRC}/${SS} ${SRC}/${DES}
	@${GCC} $< -o $@

.PHONY: all clean runClient runAS runSS runTGS
clean:
	-@rm client TGS AS SS

runClient: client
	@./$< ${KEY_CLIENT}

runAS: AS
	@./$<

runSS: SS
	@./$<

runTGS: TGS
	@./$<