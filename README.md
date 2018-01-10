# XStudio

XStudio is a game base library. that very useful library to help build game server framework. this code is accumulated five years from my work.

## Common
this library has much more useful code snippet like singleton, thread pool, http request, md5, sha1, file watcher and so on.

## Netbase
this is a network library package from asio.

## Database
this is a package for mysql. if you well, you can translate to mssql quickly.

# Feature

cross platform on windows and linux

X Game Core�������������

# ���
	X Game Core ��� XGC ���һ����µ�һ�׿�ƽ̨�ķ�����������ܡ��ÿ�ܰ������¼�������
- common  
ͨ�õײ�⣬�ں������Ϸ�������������õ����
- net  
�����������������asio��װ���ɡ�
- net_module  
������ģ�飬����Net�ĸ��ḻ�İ�װ�����ڿ��ٽ�������������������ӡ�������GateServer��GameServer֮��ͨ��һ��Sock����������������·��
- database  
���ݿ���������ڿ�������MySql���ݿ������
- nosql  
Redis��C++��װ��
- core  
������Ϸ���߼��ķ�װ
- framework  
һ�����������ģ�飬��װ��һЩ���õķ���������

## Common ͨ�õײ��
----
�ļ� | ˵��
----:|:----
auto_handle.h	| �Զ��б�
csv_reader.h	| csv �ļ���ȡ��װ
Ini_reader.h	| ini �ļ���ȡ��װ
datetime.h		| ʱ�����ں�ʱ��Ƭ�ķ�װ
debuger.h		| ���Լ�����صķ�װ
encoding.h		| utf8 �C gbk�ַ�����ת��
exception.h 	| ��Ҫ����windows�µ��쳣����
filewatcher.h 	| �ļ���������������ڼ����ļ��Ķ���
freelist.h		| �����б�ķ�װ
geometry.h		| size��rect��point�ķ�װ
getopt 			| �����н���
http.h			| windows�µ�HTTP�ͻ��ˣ�ʹ��wininet API ��д
logger.h		| һ����־ϵͳ
md5.h			| MD5У����ķ�װ��
pugixml.hpp		| һ��xml��ȡ��װ����ȡ�Ͳ����ٶȱ�TiXml�죬ʹ�÷��㡣����������
serialization.h	| �������л���
sha1.h			| SHA1 �㷨��װ
thread_pool.h	| �̳߳ط�װ
timer.h			| һ���̶�ʱ�����Ķ�ʱ��
xbuffer.h		| ��������װ
xsystem.h		| ��ϵͳ��ص�һЩ�㷨��װ
xvariant.h		| ��������ķ�װ

### auto_handle
----
	auto_handle���Զ��б��ģʽ����ģʽ�ڶ�������ʱ�������������һ����������ڶ�������ʱ�黹��������ĺô��ǣ�������Ҫ��¼����ָ���ʱ�����ֻ��¼������������Ҫ��ʱ����ö���Ĺ�������ѯ�þ��ָ��Ķ���ָ�롣��ģʽ������Ч�ž�Ұָ�����⡣  
	�˷�װʹ�þ��Chunk�����Զ�̬����������Ч�����ڴ棬�Ҳ�Ӱ��������Ч�ʡ���ģ�������ھ��������ϡ��Ļ����С��������Ч����ʧ��  

### csv_reader
----
	csv��ʽ�ļ��Ķ�ȡ��װ�ࣨ��֧��д����֧��excel�е�Ԫ����е�������Զ�ʶ��utf8���룬�ļ������ٶȿ죬�ӿ�ʹ�÷��㡣

### ini_reader
----
	ini��ʽ�ļ��Ķ�ȡ��װ�ࣨ��֧��д�����Զ�ʶ��utf8���룬�ļ������ٶȿ죬�ӿ�ʹ�÷��㡣��ini�Ļ����ϣ������ini��ʽ������չ��
1. ���Զ����ʹ����ͬ����
2. ���Զ����ʹ����ͬ����
3. ���������ⲿ�ļ�
4. ����ʹ��%Section.Key%�����Ѷ������ֵ
5. �ļ������ٶȿ顣


### datetime
----
	datetime ������ʱ��ķ�װ�����㷨��Ҫ����ʱ����ص������ǣ�ʹ�ø������Ӽ򵥡�datetime::to_string ���Խ��洢��ʱ��ת��Ϊ�ַ�����Ҳ����ʹ��datetime::convert���ַ���תΪʱ��ֵ��

	timespan ��ʱ���ķ�װ��������datetime һ����ʱ����ص����㡣

### encoding
----
	����ķ�װ���н�Ϊ����Ľӿ�ʵ��utf8 <==> mbs �ı���ת��

### filewatcher
----
	�ļ������������ڼ����ļ�������޸ġ����ʡ�������ɾ���ȣ��������˺ϲ����ƣ�һ��ʱ���ڵ��¼��ɺϲ��ϱ���

### serialization
----
	���������л��������ڽ��������л������壬��ͨ������operator<< �����ķ�ʽ����չ���л������͡����������˻���	�������ͺ�STL���������л����ء�

### thread_pool
----
	һ���̳߳صķ�װ��ʹ��C++11 ���Ա�д���ӿ�������std::bind������ֵʹ��std::future

### timer
----
	һ���̶�ʱ�����Ķ�ʱ������Ч��Ҫ�Ȼ��ڶѵĶ�ʱ�����ߣ����ʵ���ѡ�񾫶Ⱥ����ӳٿ��Դﵽ��Ϸ������

### xbuffer
----
	��������װ�����˶��ֻ���������  
1. ������
	1. static_buffer  
	��̬���壬���ڲ��й̶���С�Ļ������������ڴ�ɷ�����ջ�ϡ�
	1. reference_buffer  
	���û��壬���������л��壬ֻ�л�������Ϣ��ָ�룬��С����
	1. shared_memory_buffer  
	�����ڴ棬ʹ��ϵͳ�����ڴ�
2. �����д
	2. separate_buffer_recorder  
	����Ļ����д������дָ�벻ʹ�û����ڴ档
	2. union_buffer_recorder  
	���ϵĻ����д������дָ��ʹ�û����ڴ�
3. ��������
	3. linear_buffer  
	���Ի���
	3. ring_buffer  
	���λ���

### xsystem
----
	ϵͳ��ص��㷨��װ

���� | ˵��
---:|:---
get_module_name | ��ȡִ���ļ����ļ���
get_module_path | ��ȡִ���ļ����ļ�·��
get_absolute_path | ��ȡ�����ִ���ļ���·��
get_relative_path | ��ȡ����Ŀ¼�Ա����������·��
is_absolute_path | �ж�һ��·���Ƿ����·��
make_dirs | ����Ŀ¼���ɵݹ鴴����
list_directory |  �г�ָ��Ŀ¼�µ�Ŀ¼���ļ����ɵݹ飩
get_process_memory_usage | ��ȡ����ʹ�õ��ڴ�
get_system_memory_usage |  ��ȡϵͳʹ�õ��ڴ�
get_process_cpu_usage | ��ȡ���̵�CPUռ�����
get_system_cpu_usage |  ��ȡϵͳ��CPUռ�����

## Core �߼��ײ�
----
### XAttribute
----
	XObject���Եķ�װ������

### XClassInfo
----
	XObject RTTI�ķ�װ

### XObject
----
	XObject��Core�Ķ����࣬������Ϸ�ڶ���Ӧ��XObject������

	XObjectʹ�����Զ��б�����ÿһ��XObject���������ڲ���ʵ����ʱ�򶼻ᱻ����һ�������ͨ���洢������������ָ�������Ч��ֹҰָ��Ĳ�����

	XObject�ṩ������ϵͳ�ĳ�ʼ������ؽӿڡ�����XObject����������඼���Է����Լ����丸������ԡ�����ϵͳ�������ԵĶ��壬���ṩ�����԰汾�Ĺ���ʹ��Ϸ����ɾ���Զ����İ汾��������֮ǰ�汾�����Դ浵��

	XObject�ṩ��һ���¼�ģ�ͣ���֤��Ϸ����������¼������ʹ�����¼�����Ȥ����������ͬʱ���¼�Ҳ��������֯�ṹ���ϲ㴫�ݡ�

### XObjectNode
----
	XObjectNode �ṩ���Ӷ���Ĺ�����

### XBag
----
	XBag ��һ����������ķ�װ���ñ�������֧�������ڲ���ʧ�ܵ�����¿��Զ��Ѳ�������Ʒ���лع���

### XGameMap
----
	XGameMap ��һ�����ӵ�ͼ�Ĺ������װ��ר�����ڹ���M * N�ĸ��ӵ�ͼ�������ж���ײ���ݣ������ӣ�����Ұ����m * nһ�飩���������֡�ʹ���������Ұ�����ϸ�Ϊ��Ч��

	XGameMap ��������һ�׿����õ��¼�ϵͳ�������������Ͷ�ʱ�������XObject���¼�ģ�ͣ��������ó����ӵĵ�ͼ�淨��

	XGameMap Ҳ�ṩ�˶��ֵ�ͼ�ڶ���ļ�ȡ�㷨�������л�����Ұ��Բ�Ρ����Ρ�Բ�����Ȼ������εļ�ȡ������

	XGameMapҲ�ṩ�˿ͻ����¼��Ĵ������ƣ����Խ�����״̬���м�¼��

## Framework ��������ܲ�
	Framework��������ܿ�ʹ��net_module����Ϊ����֧�ţ�ʹ��database����Ϊ���ݿ�֧�š�ʹ�������ļ������÷�������Ϣ�����ݿ���Ϣ�Լ�������ͨ�����

	ͨ��ʹ��Framework�ڲ�дһ�д��������½�Ϊ���׵Ĵ�������������ܹ���ʹ������Ա���Խ�ע���������߼����ʵ�֡�

### ServerBase
----
	Framework����ӿڵĶ�����ʵ�ֵĴ��붼�ڸ��ļ���

### ServerParams
----
	һ������ʽ�ı����洢���ܡ�ServerParams�ֿͻ��˺ͷ��������������Ὣ�ϴ��ı����־û�����ͬ���������ͻ��ˡ���ģ����Ҫ����ȫ�ֱ�����ͬ����

### ServerCommand
----
	������ָ��ķ�װ���ɽ��ж༶�������Խϸ��ӵ�ָ���ʹ�ñ������ܽ��м򻯡�

### ServerRefresh
----
	������ˢ�¹��ܣ���ˢ�¹���ת�����ű����ýű��޸Ľ�Ϊ�������Զ���һ��ˢ������Ҫ������Ϊ����ɫ����ʱ��ˢ�µ��ж�Ӧ���߼�������ɡ�

### ServerEvent
----
	�������¼�ģ�ͣ���Ϊͬ���¼��ύ��EmmitEvent�����첽�¼��ύ��PostEvent������Ҫ���ڳɾ�ϵͳ���ӳ�ִ�еȻ����¡�

## ����
----
	��������ѡ��
1.	�ű�ʹ��luajit
2.	�ű���C++ͨѶʹ��LuaBridge�����޸ģ�
3.	Windows���ڴ�й©���ʹ��VLD
4.	�����ײ�ʹ��asio

## ��������˵��
----
1.	Sample\networkĿ¼����һ���ļ��������Ͷ�Ӧ�Ŀͻ��ˡ���Ҫ������������ѹ�����ԣ�
2.	Sample\chatĿ¼����һ������������������������������ڲ�����������ѹ�����ԡ�
3.	Sample\serversĿ¼����һ���������ܹ�ʾ��������gate_server, game_server, ��client��������ʾframework��ʹ�á�
