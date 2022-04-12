#include "pch.h"
#include "TrO.h"
#include "PasswordDlg.h"
#include "afxdialogex.h"
#include "HelperFunctions.h"

IMPLEMENT_DYNAMIC(PasswordDlg, CDialog)

PasswordDlg::PasswordDlg(CWnd* pParent /*=nullptr*/): CDialog(IDD_PASSWD, pParent){}
PasswordDlg::~PasswordDlg(){}

void PasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(PasswordDlg, CDialog)
	ON_BN_CLICKED(IDOK, &PasswordDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &PasswordDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

void PasswordDlg::OnBnClickedOk()
{
	CString inputPass;
	GetDlgItem(IDC_EDIT1)->GetWindowText(inputPass);
	CString md5str = getMD5(inputPass);
	myPass.password = inputPass;
	if (myPass.Read())
	{
		// ��������� � �� ������� ������ ("MD5 ������" � "���������� ��������� ������� ������")
		// ������� md5str � "MD5 ������". ���� ���������:
		if (myPass.hash.Compare(md5str.GetString()) == 0){			
			myPass.badlogin = 0;			
			myPass.Write();
			myPass.freeXLS();
			pAllData->setEncriptionKey(myPass.password);
			CDialog::OnOK();
		}
		else {// ���� �� ���������:
			if (myPass.hash.IsEmpty()) { // ������� ������
				myPass.hash = md5str;
				myPass.Write();
				myPass.freeXLS();
				pAllData->setEncriptionKey(_T(""));
				pAllData->connectXLS();
				pAllData->Read();
				pAllData->setEncriptionKey(myPass.password);
				pAllData->Write();
				pAllData->freeXLS();
				AfxMessageBox(_T("������ ������ �����������.\n���������� � ��������� ��� ������� �� ������������."));
				CDialog::OnCancel();
				return;
			}
			//   ��������� � �� � �� � ���������� ������� �� 1, 
			myPass.badlogin += 1;
			if (myPass.badlogin < 5) {
				myPass.Write();
				AfxMessageBox(_T("���i���� ������."));
				return;
			}			
			else {// ���� ������� ����� ����� 5, ������������� ��� �� ��������� ������
 				myPass.freeXLS();// ��������� ����
				pAllData->connectXLS();
				pAllData->setEncriptionKey(gen_random(32));// ������� ��������� �������� �� 32
				pAllData->Read();
				pAllData->setEncriptionKey(gen_random(32));// ������� ��� ���� ��������� �������� �� 32
				pAllData->Write();
				pAllData->freeXLS();
				AfxMessageBox(_T("������ ���� ������� 5 ���i� ����i�� ���i���.\n��i ����i �������."));
				CDialog::OnCancel();
			}			
		}
	}
	else {
		myPass.freeXLS();
		CDialog::OnCancel();
	}
}


void PasswordDlg::OnBnClickedCancel()
{
	myPass.freeXLS();
	CDialog::OnCancel();
}
