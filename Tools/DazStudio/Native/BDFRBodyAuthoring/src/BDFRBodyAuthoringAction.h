#pragma once

#include <dzaction.h>

class BDFRBodyAuthoringDialog;

class BDFRBodyAuthoringAction : public DzAction
{
    Q_OBJECT
public:
    BDFRBodyAuthoringAction();
    virtual ~BDFRBodyAuthoringAction();

protected:
    virtual void executeAction();

private:
    BDFRBodyAuthoringDialog* m_dialog;
};
