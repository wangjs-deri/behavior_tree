#include <qfiledialog.h>
#include <qdockwidget.h>
#include <btree/nodes.h>
#include <graph/tree_instance.h>
#include <choice_manager.h>
#include <btree/btree_config.h>
#include <dialogs/line_dialog.h>
#include <ui_debugger_main_window.h>
#include <filesystem>

#include <dialogs/config_dialog.h>


#include "debugger_main_window.h"
#include "log_viewer/log_dialog.h"

using namespace spiritsaway::behavior_tree::editor;

debugger_main_window::debugger_main_window(QWidget* parent)
	: multi_instance_window(parent)
	, ui(new Ui::debugger_main_window)
{
	init_widgets();
	init_actions();
}

void debugger_main_window::init_widgets()
{
	ui->setupUi(this);
	cur_mdi = ui->mdiArea;
	cur_mdi->setViewMode(QMdiArea::TabbedView);
	cur_mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	cur_mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	cur_mdi->setTabsMovable(true);
	cur_mdi->setTabsClosable(true);
	cur_mdi->setTabShape(QTabWidget::Rounded);
	connect(cur_mdi, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(sub_window_activated(QMdiSubWindow*)));

	QDockWidget *cur_docker_widget = new QDockWidget(tr("log_viewer"), this);
	_log_viewer = new log_dialog(_btree_cmds, this);

	//auto temp_layout = new QVBoxLayout();
	//temp_layout->setAlignment(Qt::AlignTop);
	//temp_layout->addWidget(_log_viewer);
	cur_docker_widget->setWidget(_log_viewer);
	cur_docker_widget->setFeatures(QDockWidget::DockWidgetMovable| QDockWidget::DockWidgetFloatable);
	cur_docker_widget->setMinimumWidth(400);

	addDockWidget(Qt::RightDockWidgetArea, cur_docker_widget);
}

debugger_main_window::~debugger_main_window()
{
	delete ui;
}
bool debugger_main_window::is_read_only() const
{
	return true;
}

void debugger_main_window::init_actions()
{
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(action_open_handler()));

	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(action_close_handler()));
	connect(ui->actionCloseAll, SIGNAL(triggered()), this, SLOT(action_close_all_handler()));
	connect(ui->actionGoto, SIGNAL(triggered()), this, SLOT(action_goto_handler()));
	connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(action_find_handler()));
}
bool debugger_main_window::focus_on(const std::string& tree_name, std::uint32_t node_idx)
{
	const auto& cur_btree_config = spiritsaway::behavior_tree::editor::btree_config::instance();
	std::filesystem::path cur_file_path = cur_btree_config.btree_folder / tree_name;
	std::string cur_file_path_str = cur_file_path.string();
	auto opt_ins_idx = already_open(cur_file_path_str);
	tree_instance* cur_ins;
	if (opt_ins_idx)
	{
		cur_ins = _instances[opt_ins_idx.value()];
		cur_mdi->setActiveSubWindow(cur_ins->window);
	}
	else
	{
		node* cur_root = node::load(cur_file_path_str, _logger);
		if (!cur_root)
		{
			return false;
		}
		cur_ins = new tree_instance(cur_file_path_str, cur_root, this);
	}
	cur_ins->focus_on(node_idx);
	return true;
}