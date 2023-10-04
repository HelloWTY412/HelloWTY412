#include "pch.h"
#include "Command.h"

CCommand::CCommand():threadid(0)
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = {
		{1,&CCommand::MakeDriverInfo},
		{2,&CCommand::MakeDirctoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownLoadFile},
		{5,&CCommand::MouseEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnlockMachine},
		{9,&CCommand::DeleteLocalFile},
		{1981,&CCommand::TextConnect},
		{-1,}
	};
	for (size_t i = 0; i < data[i].nCmd; i++)
	{
		m_mapFunction.insert(pair<int,CMDFUNC>(data[i].nCmd,data[i].func));
	}
}

int CCommand::ExcuteCommand(int nCmd)
{
	map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	if (it == m_mapFunction.end()) {//Ã»ÕÒµ½ÃüÁî
		return -1;
	}
	return (this->*it->second)();
}

