enum DCI_CMD{
	DCI_NO_COMMAND =0,
	DCI_SAVE_DRV_REG =1,

};//DCI ����
enum space_type{
userspace =0,
defaultsapce = 1,
};//�洢�ռ�����

void DCI_Init();//DCI ��ʼ��
void DCI_DrvCommand();//DCI ��ѭ�� ��ȡ��������ز��������ò�����
int API_DCI_GET_PARAM(unsigned char *cmd,unsigned char *cmd_param,unsigned int *param0,unsigned int *param1);//��ȡ��ز��� cmdΪdci��������Ϊ����

