#include "mainwindow.h"
#include "parser.cpp"

QVector<std::pair<double, double>> findExtremums(const QVector<double>& x, const QVector<double>& y) {
    QVector<std::pair<double, double>> extremums;

    if (x.size() < 3 || y.size() < 3) {
        return extremums;
    }

    for (long long i = 1; i < y.size() - 1; ++i) {
        if (y[i] > y[i - 1] && y[i] > y[i + 1]) {
            extremums.emplace_back(x[i], y[i]);
        }
        else if (y[i] < y[i - 1] && y[i] < y[i + 1]) {
            extremums.emplace_back(x[i], y[i]);
        }
    }

    return extremums;
}

void MainWindow::buildGraph() {
    customPlot->deselectAll();
    double xBegin = customPlot->xAxis->range().lower;
    double xEnd = customPlot->xAxis->range().upper;

    const double yUpper = customPlot->yAxis->range().upper;
    const double yLower = customPlot->yAxis->range().lower;

    const double visibleRange = xEnd - xBegin;
    double h = defaultH;
    double lineSize = 3;
    
    const size_t MAX_POINTS = 3000;


    for (int i = 0; i < customPlot->graphCount(); ++i) {
        customPlot->graph(i)->data()->clear();
    }

    customPlot->setUpdatesEnabled(false);
    int graphIndex = 0;

    #pragma omp parallel for
    for (int i = 0; i < funcBoxes.size(); i++) {
        funcBoxes[i]->property("associatedWarningLabel").value<QLabel*>()->setToolTip("");
        funcBoxes[i]->property("associatedWarningLabel").value<QLabel*>()->hide();
        const QString funcText = funcBoxes[i]->toPlainText();
        if (funcText.trimmed().isEmpty()) continue;
        std::string funcStr = funcText.toStdString();
        SimpleParser parser(funcStr);
        try{
            parser.evaluate(xBegin);
        } catch (const std::exception& e) {
            funcBoxes[i]->property("associatedWarningLabel").value<QLabel*>()->show();
            funcBoxes[i]->property("associatedWarningLabel").value<QLabel*>()->setToolTip(tr(e.what()));
            continue;
        }
        if(!visibilityFuncs[i]) continue;
        QVector<double> currentX, currentY;
        QVector<QVector<double>> segmentX, segmentY;
        const QColor& graphColor = colors[i % colors.size()];
        bool isPrevThreshold = false;
        double prevValue = NAN;
        bool firstValidValueFound = false;
        
        #pragma omp parallel for
        for(double X = xBegin-h; X <= xEnd; X += h) {
            double value = parser.evaluate(X);
            if(!std::isfinite(value)) {
                continue;
            };
            if (!firstValidValueFound) {
                prevValue = value;
                firstValidValueFound = true;
                continue;
            }
            if(abs(prevValue - value) >= threshold){
                isPrevThreshold = true;
                double yVal = (value > 0) ? yUpper : yLower;

                currentX.emplace_back(X);
                currentY.emplace_back(yVal);
                segmentX.emplace_back(std::move(currentX));
                segmentY.emplace_back(std::move(currentY));
                currentX.clear();
                currentY.clear();
            } else {
                if(isPrevThreshold && !segmentX.isEmpty() && segmentX.last().size() == 1){
                    currentX.emplace_back(segmentX.last().first());
                    currentY.emplace_back(segmentY.last().first()); 
                }
                currentX.emplace_back(X);
                currentY.emplace_back(value > 0 ? std::min(value, yUpper):std::max(value, yLower));
                isPrevThreshold = false;

            }
            prevValue = value;
        }

        if (!currentX.isEmpty()) {
            segmentX.emplace_back(std::move(currentX));
            segmentY.emplace_back(std::move(currentY));
        }

        QPen pen(graphColor, 3);
        QPen pen2(graphColor, 4);

        QVector<double> extremumX, extremumY;
        for (int j = 0; j < segmentX.size(); j++) {
            if (segmentX[j].size() >= 2) {
                if(graphIndex >= customPlot->graphCount()) customPlot->addGraph();
                QCPGraph* graph = customPlot->graph(graphIndex++);
                graph->setVisible(true);
                graph->setPen(pen);
                graph->setLineStyle(QCPGraph::lsLine);
                graph->setScatterStyle(QCPScatterStyle::ssNone);
                graph->setData(segmentX[j], segmentY[j], true);
                if(visibleRange < 1e3){
                    auto extremums = findExtremums(segmentX[j], segmentY[j]);
                    for (const auto& extremum : extremums) {
                        extremumX.push_back(extremum.first);
                        extremumY.push_back(extremum.second);
                    }
                }
            }
        }


        if (!extremumX.isEmpty()) {
            if(graphIndex >= customPlot->graphCount()) customPlot->addGraph();
            QCPGraph* extremumGraph = customPlot->graph(graphIndex++);
            extremumGraph->setVisible(true);
            extremumGraph->setPen(pen2);
            extremumGraph->setLineStyle(QCPGraph::lsNone);
            extremumGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
            extremumGraph->setData(extremumX, extremumY);
        }
        
    }

    for (; graphIndex < customPlot->graphCount(); ++graphIndex) {
        customPlot->graph(graphIndex)->setVisible(false);
    }
    
    customPlot->setUpdatesEnabled(true);
    customPlot->replot(QCustomPlot::rpQueuedReplot);
}