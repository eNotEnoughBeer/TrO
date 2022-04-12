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
		// прочитать в БД таблицу пароля ("MD5 пароля" и "количество неудачных попыток подряд")
		// сверить md5str с "MD5 пароля". если совпадает:
		if (myPass.hash.Compare(md5str.GetString()) == 0){			
			myPass.badlogin = 0;			
			myPass.Write();
			myPass.freeXLS();
			pAllData->setEncriptionKey(myPass.password);
			CDialog::OnOK();
		}
		else {// если не совпадает:
			if (myPass.hash.IsEmpty()) { // создаем пароль
				myPass.hash = md5str;
				myPass.Write();
				myPass.freeXLS();
				pAllData->setEncriptionKey(_T(""));
				pAllData->connectXLS();
				pAllData->Read();
				pAllData->setEncriptionKey(myPass.password);
				pAllData->Write();
				pAllData->freeXLS();
				AfxMessageBox(_T("Пароль успішно встановлено.\nПерезайдіть у программу для початку її використання."));
				CDialog::OnCancel();
				return;
			}
			//   увеличить в БД а не в оперативке счетчик на 1, 
			myPass.badlogin += 1;
			if (myPass.badlogin < 5) {
				myPass.Write();
				AfxMessageBox(_T("Невiрний пароль."));
				return;
			}			
			else {// если счетчик будет равен 5, перешифровать всю БД рандомным ключом
 				myPass.freeXLS();// отпустить файл
				pAllData->connectXLS();
				pAllData->setEncriptionKey(gen_random(32));// сделать рандомайз символов на 32
				pAllData->Read();
				pAllData->setEncriptionKey(gen_random(32));// сделать еще один рандомайз символов на 32
				pAllData->Write();
				pAllData->freeXLS();
				AfxMessageBox(_T("Пароль було введено 5 разiв поспiль невiрно.\nУсi даннi знищено."));
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
