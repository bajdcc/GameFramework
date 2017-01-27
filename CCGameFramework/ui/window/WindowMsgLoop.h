#ifndef UI_WINDOWMSGLOOP_H
#define UI_WINDOWMSGLOOP_H

#include <ui/gdi/Gdi.h>

class WindowMsgLoop
{
public:
    WindowMsgLoop();

    void Run();
    BOOL PumpMessage();
    BOOL IsIdleMessage(MSG* pMsg);
    static BOOL OnIdle(LONG lCount);

    using MSGMAP = std::unordered_map<UINT, CString>;
    LPCTSTR DebugGetMessageName(UINT message);

private:
    int m_nDisablePumpCount;
    UINT m_nMsgLast;
    MSG m_msg;
    CPoint m_ptMousePos;
    MSGMAP m_msgMap;
};

#endif