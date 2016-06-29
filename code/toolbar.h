#ifndef TOOLBAR
#define TOOLBAR

#include <wx/wx.h>

class Toolbar : public wxFrame
{
public:
    Toolbar(const wxString& title);

    void OnQuit(wxCommandEvent& event);


};

#endif