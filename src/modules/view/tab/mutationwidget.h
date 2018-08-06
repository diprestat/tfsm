#ifndef MUTATIONWIDGET_H
#define MUTATIONWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QtSvg>

#include "svgview.h"

#include "../../model/structs.h"

class MutationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MutationWidget(QWidget *parent = 0);
private:
    QGridLayout *_main_layout;
    QGridLayout *_test_layout;
    QPushButton *_import_button;
    QPushButton *_export_button;
    QPushButton *_ce_button;
    QPushButton *_cs_button;
    QPlainTextEdit *_test_results_text;
    QLabel *_test_results_label;
    QLabel *_nbStates_label;
    QGroupBox *_test_results_group;
    QGroupBox *_machine_content;
    QLineEdit *_nbStates_input;
    QComboBox *_machine_type;
    QTableWidget *_specification_machine_tab;
    QTableWidget *_mutation_machine_tab;
    QTableWidget *_input_tab;
    QTableWidget *_output_tab;
    SvgView *_renderer;
    void buildInterface();
    void fillInterface();
    void relaySignals();
    void updateTab();
    void setButtonStyle(QPushButton *);
signals:
    void importFile();
    void exportFile();
    void checkingExperiment();
    void checkingSequence();
public slots:
    void checkingExperimentResults(std::vector<sequence> E);
    void checkingSequenceResults(sequence s);
};

#endif // MUTATIONWIDGET_H