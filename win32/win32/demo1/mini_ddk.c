//_stdcall
/*
ntʽ������ͷ�ļ�����ntddk.h
DriverEntry ��ں��� �൱��mian���� DriverEntry������������
PDRIVER_OBJECT �˽ṹ�������������Ķ��� ��I/O���������ݽ�������������
PUNICODE_STRING �˽ṹ����ָ������������ע��� Ҳ��������������ע����е�·��
*/
#include <ntddk.h>
#define   INITCODE code_seg("INIT")
#pragma INITCODE
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject);//ǰ��˵�� ж������
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) //typedef long NTSTAUS
{
	KdPrint(("�����ɹ�������_____________OK"));   //DgbPrint("Hello DDK");
	pDriveObject->DriverUnload=DDK_Unload;
	return STATUS_SUCCESS;
}
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("�����ɹ���ж��____________OK")); //sprintf,printf
}
/*
�����е�Ԥ����ָ���У�#pragma ָ���������ӵ��ˣ������������趨��������״̬������ָʾ���������һЩ�ض��Ķ�����#pragma ָ���ÿ��������������һ������,�ڱ�����C��C++������ȫ���ݵ������,�������������ϵͳר�е����������ݶ���,����ָʾ�ǻ��������ϵͳר�е�, �Ҷ���ÿ�����������ǲ�ͬ�ġ� 
���ʽһ��Ϊ: #Pragma Para 
����Para Ϊ��������������һЩ���õĲ����� 

(

�ڱ�д�����ʱ��,���Ǿ���Ҫ�õ�#pragmaָ�����趨��������״̬������ָʾ���������һЩ�ض��Ķ���.
���������һ�¸�ָ���һЩ���ò���,ϣ���Դ����������!

һ. message ������
message
���ܹ��ڱ�����Ϣ�����
���������Ӧ����Ϣ�������Դ������Ϣ�Ŀ����Ƿǳ���Ҫ�ġ�

��ʹ�÷���Ϊ��     #pragma message("��Ϣ�ı�")

����������������ָ��ʱ���ڱ�����������н���Ϣ�ı���ӡ������
�������ڳ����ж���������������Դ����汾��ʱ�������Լ��п��ܶ���������û����ȷ��������Щ�꣬��ʱ���ǿ���������ָ���ڱ����ʱ��ͽ��м�顣��������ϣ���ж��Լ���û����Դ�����ʲô�ط�������_X86��������������ķ�����
#ifdef _X86
#pragma message("_X86 macro activated!")
#endif
�����Ƕ�����_X86������Ժ�Ӧ�ó����ڱ���ʱ�ͻ��ڱ��������������ʾ
"_X86 macro activated!"
���������ǾͲ�����Ϊ���ǵ��Լ������һЩ�ض��ĺ��ץ�������ˡ�

��. ��һ��ʹ�õñȽ϶��#pragma������code_seg��
��ʽ�磺

#pragma code_seg( [ [ { push | pop}, ] [ identifier, ] ] [ "segment-name" [, "segment-class" ] )
��ָ������ָ��������.obj�ļ��д�ŵĽ�,�۲�OBJ�ļ�����ʹ��VC�Դ���dumpbin�����г���,������.obj�ļ���Ĭ�ϵĴ�Ž�Ϊ.text�ڣ����code_segû�д������Ļ�,���������.text���С�

push (��ѡ����) ��һ����¼�ŵ��ڲ��������Ķ�ջ��,��ѡ��������Ϊһ����ʶ�����߽���
pop(��ѡ����) ��һ����¼�Ӷ�ջ���˵���,�ü�¼����Ϊһ����ʶ�����߽���
identifier (��ѡ����) ��ʹ��pushָ��ʱ,Ϊѹ���ջ�ļ�¼ָ�ɵ�һ����ʶ��,���ñ�ʶ����ɾ����ʱ�������صĶ�ջ�еļ�¼����������ջ
"segment-name" (��ѡ����) ��ʾ������ŵĽ���
����:
//Ĭ�������,�����������.text����
void func1() {                   // stored in .text
}

//�����������.my_data1����
#pragma code_seg(".my_data1")
void func2() {                   // stored in my_data1
}

//r1Ϊ��ʶ��,����������.my_data2����
#pragma code_seg(push, r1, ".my_data2")
void func3() {                   // stored in my_data2
}

int main() {
}

��. #pragma once (�Ƚϳ��ã�
����һ���Ƚϳ��õ�ָ��,ֻҪ��ͷ�ļ����ʼ��������ָ����ܹ���֤ͷ�ļ�������һ��

��. #pragma hdrstop��ʾԤ����ͷ�ļ�����Ϊֹ�������ͷ�ļ�������Ԥ���롣
BCB����Ԥ����ͷ�ļ��Լӿ����ӵ��ٶȣ����������ͷ�ļ�������Ԥ�����ֿ���ռ̫����̿ռ䣬����ʹ�����ѡ���ų�һЩͷ�ļ���
��ʱ��Ԫ֮����������ϵ�����絥ԪA������ԪB�����Ե�ԪBҪ���ڵ�ԪA���롣�������#pragma startupָ���������ȼ������ʹ����#pragma package(smart_init) ��BCB�ͻ�������ȼ��Ĵ�С�Ⱥ���롣

��. #pragma warningָ��
��ָ��������ѡ���Ե��޸ı������ľ�����Ϣ����Ϊ
ָ���ʽ����:
#pragma warning( warning-specifier : warning-number-list [; warning-specifier : warning-number-list...]
#pragma warning( push[ ,n ] )
#pragma warning( pop )

��Ҫ�õ��ľ����ʾ�����¼���:

once:ֻ��ʾһ��(����/�����)��Ϣ
default:���ñ������ľ�����Ϊ��Ĭ��״̬
1,2,3,4:�ĸ����漶��
disable:��ָֹ���ľ�����Ϣ
error:��ָ���ľ�����Ϣ��Ϊ���󱨸�

�����Ҷ�����Ľ��Ͳ��Ǻ����,���Բο�һ����������Ӽ�˵��

#pragma warning( disable : 4507 34; once : 4385; error : 164 )
�ȼ��ڣ�
#pragma warning(disable:4507 34)   // ����ʾ4507��34�ž�����Ϣ
#pragma warning(once:4385)         // 4385�ž�����Ϣ������һ��
#pragma warning(error:164)         // ��164�ž�����Ϣ��Ϊһ������
ͬʱ���pragma warning Ҳ֧�����¸�ʽ��
#pragma warning( push [ ,n ] )
#pragma warning( pop )
����n����һ������ȼ�(1---4)��
#pragma warning( push )�������о�����Ϣ�����еľ���״̬��
#pragma warning( push, n)�������о�����Ϣ�����еľ���״̬�����Ұ�ȫ�־���ȼ��趨Ϊn��
#pragma warning( pop )��ջ�е������һ��������Ϣ������ջ�ͳ�ջ֮��������һ�иĶ�ȡ�������磺
#pragma warning( push )
#pragma warning( disable : 4705 )
#pragma warning( disable : 4706 )
#pragma warning( disable : 4707 )
#pragma warning( pop )

����δ����������±������еľ�����Ϣ(����4705��4706��4707)

��ʹ�ñ�׼C++���б�̵�ʱ�򾭳���õ��ܶ�ľ�����Ϣ,����Щ������Ϣ���ǲ���Ҫ����ʾ,�������ǿ���ʹ��#pragma warning(disable:4786)����ֹ�����͵ľ�����vc��ʹ��ADO��ʱ��Ҳ��õ�����Ҫ�ľ�����Ϣ,���ʱ�����ǿ���ͨ��#pragma warning(disable:4146)�����������͵ľ�����Ϣ

��. pragma comment(...)
��ָ��ĸ�ʽΪ��   #pragma comment( "comment-type" [, commentstring] )
��ָ�һ��ע�ͼ�¼����һ�������ļ����ִ���ļ���,comment-type(ע������):����ָ��Ϊ����Ԥ����ı�ʶ��������һ�֡�
����Ԥ����ı�ʶ��Ϊ:

1��compiler:���������İ汾�ź����Ʒ���Ŀ���ļ���,����ע�ͼ�¼��������������
�����Ϊ�ü�¼�����ṩ��commentstring����,�������������һ������
����:#pragma comment( compiler )

2��exestr:��commentstring��������Ŀ���ļ���,�����ӵ�ʱ������ַ����������뵽��ִ���ļ���,������ϵͳ���ؿ�ִ���ļ���ʱ��,�ò����ַ������ᱻ���ص��ڴ���.����,���ַ������Ա�dumpbin֮��ĳ�����ҳ�����ӡ����,������������ʶ�����汾����֮�����ϢǶ�뵽��ִ���ļ���!

3��lib:����һ���ǳ����õĹؼ���,������һ�����ļ����ӵ�Ŀ���ļ��г��õ�lib�ؼ��֣����԰���������һ�����ļ���
����: 
#pragma comment(lib, "user32.lib")
��ָ��������user32.lib���ļ����뵽��������

4��linker:��һ������ѡ�����Ŀ���ļ���,�����ʹ�����ָ���������������д���Ļ����ڿ������������õ�����ѡ��,�����ָ��/includeѡ����ǿ�ư���ĳ������,����:
#pragma comment(linker, "/include:__mySymbol")
������ڳ�����������������ѡ��
/DEFAULTLIB
/EXPORT
/INCLUDE
/MERGE
/SECTION

��Щѡ��������Ͳ�һһ˵����,��ϸ��Ϣ�뿴msdn!

5��user:��һ���ע����Ϣ����Ŀ���ļ���commentstring��������ע�͵��ı���Ϣ,���ע�ͼ�¼��������������
����:
#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ )
*/
