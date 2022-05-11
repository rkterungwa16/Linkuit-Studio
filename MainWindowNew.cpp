#include "MainWindowNew.h"
#include "ui_MainWindowNew.h"

MainWindowNew::MainWindowNew(QWidget *pParent) :
    QMainWindow(pParent),
    mUi(new Ui::MainWindowNew),
    mView(mCoreLogic),
    mCoreLogic(mView)
{
    mUi->setupUi(this);

    mAwesome = new QtAwesome(this);
    mAwesome->initFontAwesome();
    mAwesome->setDefaultOption("color", QColor(0, 204, 143));
    mAwesome->setDefaultOption("color-disabled", QColor(100, 100, 100));
    mAwesome->setDefaultOption( "color-active", QColor(0, 204, 143));
    mAwesome->setDefaultOption( "color-selected", QColor(0, 204, 143));

    mScene.setSceneRect(canvas::DIMENSIONS);
    mView.SetScene(mScene);

    mUi->uViewLayout->addWidget(&mView, 0, 1);

    InitializeToolboxTree();

    InitializeGlobalShortcuts();
}

MainWindowNew::~MainWindowNew()
{
    delete mUi;
}

void MainWindowNew::InitializeToolboxTree()
{
    QObject::connect(mUi->uToolboxTree, &QTreeView::pressed, this, &MainWindowNew::OnToolboxTreeClicked);

    // Create category and root level items
    mCategoryGatesItem = new QStandardItem(mAwesome->icon(fa::folder), "Gates");
    mCategoryGatesItem->setSelectable(false);
    mToolboxTreeModel.appendRow(mCategoryGatesItem);

    auto textLabelItem = new QStandardItem(mAwesome->icon(fa::font), "Text label");
    mToolboxTreeModel.appendRow(textLabelItem);

#warning idea: extend fields for configurations on select

    // Create component items
    mCategoryGatesItem->appendRow(new QStandardItem(mAwesome->icon(fa::microchip), "AND gate"));
    mCategoryGatesItem->appendRow(new QStandardItem(mAwesome->icon(fa::microchip), "OR gate"));
    mCategoryGatesItem->appendRow(new QStandardItem(mAwesome->icon(fa::microchip), "XOR gate"));
    mCategoryGatesItem->appendRow(new QStandardItem(mAwesome->icon(fa::microchip), "NOT gate"));
    mCategoryGatesItem->appendRow(new QStandardItem(mAwesome->icon(fa::microchip), "Buffer gate"));

    mUi->uToolboxTree->setModel(&mToolboxTreeModel);
    mUi->uToolboxTree->setExpanded(mCategoryGatesItem->index(), true);
}

void MainWindowNew::InitializeGlobalShortcuts()
{
    // Shortcuts to change component input count
    mOneGateInputShortcut    = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this);
    mTwoGateInputsShortcut   = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this);
    mThreeGateInputsShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this);
    mFourGateInputsShortcut  = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_4), this);
    mFiveGateInputsShortcut  = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_5), this);
    mSixGateInputsShortcut   = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_6), this);
    mSevenGateInputsShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_7), this);
    mEightGateInputsShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_8), this);
    mNineGateInputsShortcut  = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_9), this);

    mOneGateInputShortcut->setAutoRepeat(false);
    mTwoGateInputsShortcut->setAutoRepeat(false);
    mThreeGateInputsShortcut->setAutoRepeat(false);
    mFourGateInputsShortcut->setAutoRepeat(false);
    mFiveGateInputsShortcut->setAutoRepeat(false);
    mSixGateInputsShortcut->setAutoRepeat(false);
    mSevenGateInputsShortcut->setAutoRepeat(false);
    mEightGateInputsShortcut->setAutoRepeat(false);
    mNineGateInputsShortcut->setAutoRepeat(false);

    QObject::connect(mOneGateInputShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(1);
    });
    QObject::connect(mTwoGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(2);
    });
    QObject::connect(mThreeGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(3);
    });
    QObject::connect(mFourGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(4);
    });
    QObject::connect(mFiveGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(5);
    });
    QObject::connect(mSixGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(6);
    });
    QObject::connect(mSevenGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(7);
    });
    QObject::connect(mEightGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(8);
    });
    QObject::connect(mNineGateInputsShortcut, &QShortcut::activated, this, [&]()
    {
       SetComponentInputCountIfInAddMode(9);
    });

    // Shortcuts to change component direction
    mComponentDirectionRightShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right), this);
    mComponentDirectionDownShortcut  = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down), this);
    mComponentDirectionLeftShortcut  = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left), this);
    mComponentDirectionUpShortcut    = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up), this);

    mComponentDirectionRightShortcut->setAutoRepeat(false);
    mComponentDirectionDownShortcut->setAutoRepeat(false);
    mComponentDirectionLeftShortcut->setAutoRepeat(false);
    mComponentDirectionUpShortcut->setAutoRepeat(false);

    QObject::connect(mComponentDirectionRightShortcut, &QShortcut::activated, this, [&]()
    {
        SetComponentDirectionIfInAddMode(Direction::RIGHT);
    });
    QObject::connect(mComponentDirectionDownShortcut, &QShortcut::activated, this, [&]()
    {
        SetComponentDirectionIfInAddMode(Direction::DOWN);
    });
    QObject::connect(mComponentDirectionLeftShortcut, &QShortcut::activated, this, [&]()
    {
        SetComponentDirectionIfInAddMode(Direction::LEFT);
    });
    QObject::connect(mComponentDirectionUpShortcut, &QShortcut::activated, this, [&]()
    {
        SetComponentDirectionIfInAddMode(Direction::UP);
    });

    mCopyShortcut  = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C), this);
    mPasteShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_V), this);

    mCopyShortcut->setAutoRepeat(false);
    mPasteShortcut->setAutoRepeat(false);

    QObject::connect(mCopyShortcut, &QShortcut::activated, this, [&]()
    {
#warning temporary implementation of Ctrl-C
        mCoreLogic.CopySelectedComponents();
    });
    QObject::connect(mPasteShortcut, &QShortcut::activated, this, [&]()
    {
        qDebug() << "Not implemented";
    });

    mSaveShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    mOpenShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this);

    mSaveShortcut->setAutoRepeat(false);
    mOpenShortcut->setAutoRepeat(false);

    QObject::connect(mSaveShortcut, &QShortcut::activated, this, [&]()
    {
        qDebug() << "Not implemented";
    });
    QObject::connect(mOpenShortcut, &QShortcut::activated, this, [&]()
    {
        qDebug() << "Not implemented";
    });

    mUndoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this);
    mRedoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y), this);

    mUndoShortcut->setAutoRepeat(true);
    mRedoShortcut->setAutoRepeat(true);

    QObject::connect(mUndoShortcut, &QShortcut::activated, this, [&]()
    {
        mCoreLogic.Undo();
    });
    QObject::connect(mRedoShortcut, &QShortcut::activated, this, [&]()
    {
        mCoreLogic.Redo();
    });

    mSimulationShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);

    mSimulationShortcut->setAutoRepeat(false);

    QObject::connect(mSimulationShortcut, &QShortcut::activated, this, [&]()
    {
        if (mCoreLogic.IsSimulationRunning())
        {
            mCoreLogic.EnterControlMode(ControlMode::EDIT);
        }
        else
        {
            mCoreLogic.EnterControlMode(ControlMode::SIMULATION);
        }
    });

    mDeleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);

    mDeleteShortcut->setAutoRepeat(false);

    QObject::connect(mDeleteShortcut, &QShortcut::activated, this, [&]()
    {
        if (!mCoreLogic.IsSimulationRunning())
        {
            mCoreLogic.DeleteSelectedComponents();
        }
    });

    mEscapeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);

    mEscapeShortcut->setAutoRepeat(false);

    QObject::connect(mEscapeShortcut, &QShortcut::activated, this, [&]()
    {
        mCoreLogic.EnterControlMode(ControlMode::EDIT);
        mScene.clearSelection();
#warning clear selection on ControlModeChangedSignal
        mUi->uToolboxTree->clearSelection();
    });
}

void MainWindowNew::SetComponentInputCountIfInAddMode(uint8_t pCount)
{
    if (mCoreLogic.GetControlMode() == ControlMode::ADD)
    {
        mCoreLogic.SetComponentInputCount(pCount);
    }
}

void MainWindowNew::SetComponentDirectionIfInAddMode(Direction pDirection)
{
    if (mCoreLogic.GetControlMode() == ControlMode::ADD)
    {
        mCoreLogic.SetComponentDirection(pDirection);
    }
}

View& MainWindowNew::GetView()
{
    return mView;
}

CoreLogic& MainWindowNew::GetCoreLogic()
{
    return mCoreLogic;
}

void MainWindowNew::OnToolboxTreeClicked(const QModelIndex &pIndex)
{
    qDebug() << pIndex.parent().row();
    qDebug() << pIndex.row();

    switch(pIndex.parent().row())
    {
        case -1: // Root level
        {
            switch(pIndex.row())
            {
                case 1: // Text label
                {
                    mCoreLogic.EnterAddControlMode(ComponentType::TEXT_LABEL);
                    break;
                }
                default:
                {
                    qDebug() << "Unknown item in root level";
                    mCoreLogic.EnterControlMode(ControlMode::EDIT);
                    mScene.clearSelection();
                    break;
                }
            }
            break;
        }
        case 0: // Gates
        {
            switch(pIndex.row())
            {
                case 0: // AND gate
                {
                    mCoreLogic.EnterAddControlMode(ComponentType::AND_GATE);
                    break;
                }
                case 1: // OR gate
                {
                    mCoreLogic.EnterAddControlMode(ComponentType::OR_GATE);
                    break;
                }
                case 2: // XOR gate
                {
                    mCoreLogic.EnterAddControlMode(ComponentType::XOR_GATE);
                    break;
                }
                case 3: // NOT gate
                {
                    mCoreLogic.EnterAddControlMode(ComponentType::NOT_GATE);
                    break;
                }
                case 4: // Buffer gate
                {
                    mCoreLogic.EnterAddControlMode(ComponentType::BUFFER_GATE);
                    break;
                }
                default:
                {
                    qDebug() << "Unknown gate";
                    break;
                }
            }
            break;
        }
        default:
        {
            mCoreLogic.EnterControlMode(ControlMode::EDIT);
            mScene.clearSelection();
            break;
        }
    }
}

