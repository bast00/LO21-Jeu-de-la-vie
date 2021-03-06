#include "headers/fenautomate2d.h"



fenAutomate2D::fenAutomate2D(QString nom, Simulateur* s):fenAutomate(nom,s) {
    maGrid = new QTableWidget(s->getInitialState()->getNbRows(),s->getInitialState()->getNbCols());
    monLayout->addWidget(maGrid,0,0,90,100);
    maGrid->horizontalHeader()->hide();
    maGrid->verticalHeader()->hide();
    maGrid->verticalHeader()->setMinimumSectionSize(1);
    maGrid->horizontalHeader()->setMinimumSectionSize(1);


    unsigned int cols = monSimu->dernier().getNbCols();
    unsigned int rows = monSimu->dernier().getNbRows();

   for(unsigned int i(0);i<rows;i++){
        for(unsigned int j(0);j<cols;j++){
          maGrid->setItem(i,j,new QTableWidgetItem(""));
          maGrid->item(i,j)->setFlags(Qt::ItemIsEnabled);
        }
    }

    resizeGrid();
    refreshGrid();
    connect(maGrid,SIGNAL(clicked(QModelIndex)),this,SLOT(slotGridClick(QModelIndex)));
}


void fenAutomate2D::resizeGrid(){
    int size;
        unsigned int cols = maGrid->columnCount();
        unsigned int rows = maGrid->rowCount();

        if(maGrid->size().width()/cols > maGrid->size().height()/rows)
            size = (int) maGrid->size().height()/rows;
        else
            size = (int) maGrid->size().width()/cols;
        size = (size<1)? 1:size;
        for(unsigned int i(0);i<rows;i++){
           maGrid->setRowHeight(i,size);
        }
        for(unsigned int j(0);j<cols;j++){
          maGrid->setColumnWidth(j,size);
        }
}

void fenAutomate2D::cellChange(unsigned int i, unsigned int j){
    // Changement direct de l'utilisateur => Réinitialisation
    monSimu->setEtatDepart(monSimu->dernier());
    // Changement d'état déterminé par la méthode Automate->changeStatus (polymorphisme)
    monSimu->changeStatus(i,j);

    // On prends l'item de la grille cliqué, on récupère la valeur correspondante dans l'etat initial, et on affiche
    // la couleur correspondante via la fonction dédiée dans l'automate
    maGrid->item(i,j)->setBackgroundColor(monSimu->getAutomate()->colorize(monSimu->getInitialState()->getCellule(i,j)));
    monSimu->reset();
}


void fenAutomate2D::refreshGrid(){
    const Etat& dernier = monSimu->dernier();
    int cols = (int) dernier.getNbCols();
    int rows = (int) dernier.getNbRows();
    const Automate* monAuto = monSimu->getAutomate();

    for(int i(0);i<rows;i++){
        for(int j(0);j<cols;j++){
            maGrid->item(i,j)->setBackgroundColor(monAuto->colorize(dernier.getCellule(i,j)));
        }
    }
}


void fenAutomate2D::avancer(){
    monSimu->next();
    refreshGrid();
    maGrid->repaint();
}


void fenAutomate2D::reculer(){
    if(monSimu->stepBack() == true){
        refreshGrid();
        maGrid->repaint();
    }else
        QMessageBox::warning(this, "BackStep fonctionality unavailable", "Impossible de retourner en arrière !");
 }


void fenAutomate2D::slotSizeChange(){
    resizeGrid();
}


void fenAutomate2D::slotGridClick(QModelIndex j){
    cellChange(j.row(),j.column());

}


void fenAutomate2D::reinitialize(){
    for(unsigned int i(0); i<maGrid->rowCount(); ++i)
        for(unsigned int j(0); j<maGrid->columnCount(); ++j)
            maGrid->item(i,j)->setBackgroundColor("white");
    maGrid->repaint();
}

void fenAutomate2D::addCols(unsigned int c){
    unsigned int cols = maGrid->columnCount();
    maGrid->setColumnCount(cols+c);
    for(unsigned int i(0); i<maGrid->rowCount(); ++i){
        for(unsigned int j(cols); j<cols+c; ++j){
            maGrid->setItem(i,j, new QTableWidgetItem(""));
            maGrid->item(i,j)->setFlags(Qt::ItemIsEnabled);
            maGrid->item(i,j)->setBackgroundColor(monSimu->getAutomate()->colorize(monSimu->dernier().getCellule(i,j)));
        }
    }
    resizeGrid();
}

void fenAutomate2D::addRows(unsigned int r){
    unsigned int rows = maGrid->rowCount();
    maGrid->setRowCount(rows+r);
    for(unsigned int j(0); j<maGrid->columnCount(); ++j){
        for(unsigned int i(rows); i<rows+r; ++i){
            maGrid->setItem(i,j, new QTableWidgetItem(""));
            maGrid->item(i,j)->setFlags(Qt::ItemIsEnabled);
            maGrid->item(i,j)->setBackgroundColor(monSimu->getAutomate()->colorize(monSimu->dernier().getCellule(i,j)));
        }
    }
    resizeGrid();
}

void fenAutomate2D::redimensionner(){
    QWidget* widgetDimensions = new QWidget();
    QFormLayout* mainLayout = new QFormLayout();

    dimRows = new QSpinBox(widgetDimensions);
    dimRows->setRange(1,100);
    dimRows->setValue(monSimu->dernier().getNbRows());
    dimCols = new QSpinBox(widgetDimensions);
    dimCols->setRange(1,100);
    dimCols->setValue(monSimu->dernier().getNbCols());


    QPushButton* BtOk = new QPushButton("Valider");
    connect(BtOk, SIGNAL(clicked()), this, SLOT(slotUpdateDimensions()));

    QPushButton* BtCancel = new QPushButton("Annuler");
    connect(BtCancel, SIGNAL(clicked()), widgetDimensions,SLOT(close()));

    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->addWidget(BtOk);
    hlayout->addWidget(BtCancel);

    mainLayout->addRow("Nombres de lignes :", dimRows);
    mainLayout->addRow("Nombres de colonnes :", dimCols);
    mainLayout->addRow(hlayout);

    widgetDimensions->setLayout(mainLayout);
    widgetDimensions->show();
}

void fenAutomate2D::slotUpdateDimensions(){
    monSimu->setEtatDepart(monSimu->dernier());
    unsigned int newDimRows = dimRows->value();
    unsigned int newDimCols = dimCols->value();

    monSimu->redim(newDimRows, newDimCols);  // Met l'état de départ du Simu à jour
    monSimu->reset();   // Réset le simu avec l'état de départ

    if(newDimRows < maGrid->rowCount()){
        for(unsigned int i(maGrid->rowCount()-1); i>=newDimRows; --i)
            maGrid->removeRow(i);
    }
    else if(newDimRows > maGrid->rowCount())
        addRows(newDimRows - maGrid->rowCount());

    if(newDimCols < maGrid->columnCount()){
        for(unsigned int j(maGrid->columnCount()-1); j>=newDimCols; --j)
            maGrid->removeColumn(j);
    }
    else if(newDimCols > maGrid->columnCount())
        addCols(newDimCols - maGrid->columnCount());

    resizeGrid();
}
