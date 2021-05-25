#include "briefing.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace hooks
{
	void updateMissionObjectives() // 0046CD30
	{
        BinDlg* readyRoomVar = *readyRoomDialog;
        if ((*readyRoomDialog)->controlType)
        {
            readyRoomVar = (*readyRoomDialog)->parent;
        }

        BinDlg* objectiveDlg = nullptr;
        BinDlg* childDlg = readyRoomVar->childrenDlg;
        if (childDlg)
        {
            while (childDlg->index != -11)
            {
                childDlg = childDlg->next;
                if (!childDlg)
                {
                    goto LABEL_6;
                }
            }
            objectiveDlg = childDlg;
        }
        else
        {
LABEL_6:
            objectiveDlg = 0;
        }

        const char* objectiveText;
        if (mapStringTbl)
        {
            u16 mapStringID = *((s16*)&missionObjectivesStringIndex + 2 * *LOCAL_NATION_ID) - 1;
            if (*((s16*)&missionObjectivesStringIndex + 2 * *LOCAL_NATION_ID))
            {
                if (mapStringID < *(s16*)mapStringTbl)
                {
                    objectiveText = (char*)mapStringTbl + *((u16*)mapStringTbl + mapStringID + 1);
                }
                else
                {
                    objectiveText = "";
                }
            }
            else
            {
                objectiveText = 0;
            }
        }
        else
        {
            objectiveText = 0;
        }

        objectiveDlg->pszText = (char*)objectiveText;
        helper::showDialog(objectiveDlg);
        s32 objectiveDlgFlags = objectiveDlg->flags;
        if (!(objectiveDlgFlags & 1))
        {
            objectiveDlg->flags = objectiveDlgFlags | 1;
            helper:: updateDialog(objectiveDlg);
        }
	}
}