#include "stdafx.h"
#include "sim.h"

#include "bochs.h"
#include "bxversion.h"
#include "param_names.h"
#include "gui/textconfig.h"
#include "cpu/cpu.h"
#include "iodev/iodev.h"
#include "gui/win32dialog.h"

static CTraceCategoryEx<CTraceCategoryEx<>::TraceSync> atlTraceBochs(_T("atlTraceBochs"));

BxEvent* sim_notify_callback(void *unused, BxEvent *event)
{
    event->retcode = -1;
    switch (event->type)
    {
    case BX_SYNC_EVT_LOG_DLG:
        event->retcode = 0;
        return event;
    case BX_ASYNC_EVT_LOG_MSG:
        switch (event->u.logmsg.level) {
        case LOGLEV_INFO:
            ATLTRACE(atlTraceBochs, 0, "INFO  | %s", event->u.logmsg.msg);
            break;
        case LOGLEV_PANIC:
            ATLTRACE(atlTraceBochs, 0, "PANIC | %s", event->u.logmsg.msg);
            break;
        case LOGLEV_ERROR:
            ATLTRACE(atlTraceBochs, 0, "ERROR | %s", event->u.logmsg.msg);
            break;
        case LOGLEV_DEBUG:
            ATLTRACE(atlTraceBochs, 0, "DEBUG | %s", event->u.logmsg.msg);
            break;
        default: break;
        }
        event->retcode = 0;
        return event;
    case BX_SYNC_EVT_ASK_PARAM:
        event->retcode = 0;
        return event;
    case BX_SYNC_EVT_TICK: // called periodically by siminterface.
        event->retcode = 0;
        // fall into default case
    default:
        return event;
    }
}

void Sim()
{
    CStringA strExePath;
    GetModuleFileNameA(NULL, strExePath.GetBuffer(MAX_PATH), MAX_PATH);
    strExePath.ReleaseBuffer();
    strExePath = strExePath.Left(strExePath.ReverseFind('\\'));
    auto BIOS_PATH = strExePath + "\\img\\bios-bochs-latest";
    auto VGA_PATH = strExePath + "\\img\\VGABIOS-lgpl-latest";
    auto FLOPPY_PATH = strExePath + "\\img\\floppy.img";
    if (!PathFileExistsA(BIOS_PATH))
    {
        MessageBoxA(NULL, "BIOS NOT FOUND", BIOS_PATH, MB_OK);
        return;
    }
    if (!PathFileExistsA(VGA_PATH))
    {
        MessageBoxA(NULL, "VGA NOT FOUND", VGA_PATH, MB_OK);
        return;
    }
    if (!PathFileExistsA(FLOPPY_PATH))
    {
        MessageBoxA(NULL, "FLOPPY NOT FOUND", FLOPPY_PATH, MB_OK);
        return;
    }
    SAFE_GET_IOFUNC();
    SAFE_GET_GENLOG();
    plugin_startup();
    bx_init_siminterface();
    SIM->init_save_restore();
    bx_init_options();
    init_win32_config_interface();
    PLUG_load_gui_plugin("d2d");
    SIM->set_init_done(0);
    SIM->set_log_viewer(BX_TRUE);
    SIM->set_notify_callback(sim_notify_callback, NULL);
    SIM->get_param_string(BXPN_VGA_ROM_PATH)->set(VGA_PATH);
    auto base = (bx_list_c*)SIM->get_param(BXPN_FLOPPYA);
    SIM->get_param_string("path", base)->set(FLOPPY_PATH);
    SIM->get_param_enum("type", base)->set(BX_FLOPPY_1_44);
    SIM->get_param_enum("status", base)->set(BX_INSERTED);

    bx_pc_system.initialize(10000000);
    BX_MEM(0)->init_memory(0x4000000, 0x4000000);
    BX_MEM(0)->load_ROM(BIOS_PATH, 0, 0);
    BX_CPU(0)->initialize();
    BX_CPU(0)->sanity_checks();
    BX_CPU(0)->register_state();
    BX_INSTR_INITIALIZE(0);
    DEV_init_devices();
    bx_pc_system.register_state();
    DEV_register_state();
    bx_set_log_actions_by_device(1);
    bx_pc_system.Reset(BX_RESET_HARDWARE);
    bx_gui->init_signal_handlers();
    bx_pc_system.start_timers();
    SIM->set_init_done(1);
    bx_gui->update_drive_status_buttons();
    bx_gui->statusbar_setitem(-1, 0);
    while (1) {
        BX_CPU(0)->cpu_loop();
        if (bx_pc_system.kill_bochs_request)
            break;
    }
    bx_atexit();
}
