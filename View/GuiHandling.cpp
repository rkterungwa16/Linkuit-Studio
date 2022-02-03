#include "View.h"
#include "HelperFunctions.h"
#include "Components/BaseComponent.h"

#include <QtWidgets>

void View::CreateGui()
{
    QHBoxLayout *topButtonsLayout = new QHBoxLayout;

    mEditButton = new QToolButton;
    mEditButton->setText(tr("Edit"));
    mEditButton->setCheckable(true);
    mEditButton->setChecked(true);

    mDeleteButton = new QToolButton;
    mDeleteButton->setText(tr("Delete"));
    mDeleteButton->setCheckable(false);
    mDeleteButton->setChecked(false);

    mCopyButton = new QToolButton;
    mCopyButton->setText(tr("Copy"));
    mCopyButton->setCheckable(false);
    mCopyButton->setChecked(false);

    mAddAndGateButton = new QToolButton;
    mAddAndGateButton->setText(tr("And Gate"));
    mAddAndGateButton->setCheckable(true);
    mAddAndGateButton->setChecked(false);

    mAddOrGateButton = new QToolButton;
    mAddOrGateButton->setText(tr("Or Gate"));
    mAddOrGateButton->setCheckable(true);
    mAddOrGateButton->setChecked(false);

    mAddXorGateButton = new QToolButton;
    mAddXorGateButton->setText(tr("Xor Gate"));
    mAddXorGateButton->setCheckable(true);
    mAddXorGateButton->setChecked(false);

    mAddNotGateButton = new QToolButton;
    mAddNotGateButton->setText(tr("Not Gate"));
    mAddNotGateButton->setCheckable(true);
    mAddNotGateButton->setChecked(false);

    mUndoButton = new QToolButton;
    mUndoButton->setText(tr("Undo"));
    mUndoButton->setCheckable(false);
    mUndoButton->setChecked(false);

    mRedoButton = new QToolButton;
    mRedoButton->setText(tr("Redo"));
    mRedoButton->setCheckable(false);
    mRedoButton->setChecked(false);

    QButtonGroup *topButtonsGroup = new QButtonGroup(this);
    topButtonsGroup->setExclusive(true);
    topButtonsGroup->addButton(mEditButton);
    topButtonsGroup->addButton(mDeleteButton);
    topButtonsGroup->addButton(mCopyButton);
    topButtonsGroup->addButton(mAddAndGateButton);
    topButtonsGroup->addButton(mAddOrGateButton);
    topButtonsGroup->addButton(mAddXorGateButton);
    topButtonsGroup->addButton(mAddNotGateButton);
    topButtonsGroup->addButton(mUndoButton);
    topButtonsGroup->addButton(mRedoButton);

    topButtonsLayout->addStretch();
    topButtonsLayout->addWidget(mEditButton);
    topButtonsLayout->addWidget(mDeleteButton);
    topButtonsLayout->addWidget(mCopyButton);
    topButtonsLayout->addWidget(mAddAndGateButton);
    topButtonsLayout->addWidget(mAddOrGateButton);
    topButtonsLayout->addWidget(mAddXorGateButton);
    topButtonsLayout->addWidget(mAddNotGateButton);
    topButtonsLayout->addWidget(mUndoButton);
    topButtonsLayout->addWidget(mRedoButton);
    topButtonsLayout->addStretch();

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addLayout(topButtonsLayout, 0, 0);
    topLayout->addWidget(&mGraphicsView, 1, 0);
    setLayout(topLayout);

}

void View::ConnectGuiSignalsAndSlots()
{
    QObject::connect(mEditButton, &QAbstractButton::clicked, this, [&](){
        mCoreLogic.EnterControlMode(ControlMode::EDIT);
    });

    QObject::connect(mAddAndGateButton, &QAbstractButton::clicked, this, [&](){
        mCoreLogic.EnterAddControlMode(ComponentType::AND_GATE);
    });

    QObject::connect(mAddOrGateButton, &QAbstractButton::clicked, this, [&](){
        mCoreLogic.EnterAddControlMode(ComponentType::OR_GATE);
    });

    QObject::connect(mAddXorGateButton, &QAbstractButton::clicked, this, [&](){
        mCoreLogic.EnterAddControlMode(ComponentType::XOR_GATE);
    });

    QObject::connect(mAddNotGateButton, &QAbstractButton::clicked, this, [&](){
        mCoreLogic.EnterAddControlMode(ComponentType::NOT_GATE);
    });

    QObject::connect(mDeleteButton, &QAbstractButton::clicked, &mCoreLogic, &CoreLogic::DeleteSelectedComponents);
    QObject::connect(mCopyButton, &QAbstractButton::clicked, &mCoreLogic, &CoreLogic::CopySelectedComponents);
    QObject::connect(mUndoButton, &QAbstractButton::clicked, &mCoreLogic, &CoreLogic::Undo);
    QObject::connect(mRedoButton, &QAbstractButton::clicked, &mCoreLogic, &CoreLogic::Redo);
}