#include "BDFRBodyAuthoringAction.h"
#include "BDFRBodyAuthoringDialog.h"

#include <dzapp.h>
#include <dzmainwindow.h>

BDFRBodyAuthoringAction::BDFRBodyAuthoringAction()
    : DzAction(tr("BDFR Body Authoring"), tr("Author muscle and soft-tissue metadata for BDFR CharacterBridge.")),
      m_dialog(0)
{
    setObjectName("BDFR_BodyAuthoring_Action");
}

BDFRBodyAuthoringAction::~BDFRBodyAuthoringAction()
{
    if (m_dialog)
    {
        m_dialog->close();
        delete m_dialog;
        m_dialog = 0;
    }
}

void BDFRBodyAuthoringAction::executeAction()
{
    if (!m_dialog)
    {
        DzMainWindow* mainWindow = dzApp ? dzApp->getInterface() : 0;
        m_dialog = new BDFRBodyAuthoringDialog(mainWindow);
        m_dialog->setAttribute(Qt::WA_DeleteOnClose, false);
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}
