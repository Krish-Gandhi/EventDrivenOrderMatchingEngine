#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "engine.h"

namespace py = pybind11;

PYBIND11_MODULE(engine_py, m) {
    py::enum_<OrderType>(m, "OrderType")
        .value("MARKET_BUY", OrderType::MARKET_BUY)
        .value("MARKET_SELL", OrderType::MARKET_SELL)
        .value("LIMIT_BUY", OrderType::LIMIT_BUY)
        .value("LIMIT_SELL", OrderType::LIMIT_SELL)
        .value("STOP_BUY", OrderType::STOP_BUY)
        .value("STOP_SELL", OrderType::STOP_SELL)
        .value("STOP_LIMIT_BUY", OrderType::STOP_LIMIT_BUY)
        .value("STOP_LIMIT_SELL", OrderType::STOP_LIMIT_SELL)
        .value("FOK_BUY", OrderType::FOK_BUY)
        .value("FOK_SELL", OrderType::FOK_SELL)
        .export_values();

    py::class_<Order>(m, "Order")
        .def(py::init<OrderType, int, float, std::string, float>(),
             py::arg("orderType"), py::arg("orderVolume"), py::arg("pricePerShare") = 0,
             py::arg("orderVisibility") = "public", py::arg("stopPrice") = 0)
        .def("getOrderType", &Order::getOrderType)
        .def("getOrderVolume", &Order::getOrderVolume)
        .def("getPrice", &Order::getPrice)
        .def("getStopPrice", &Order::getStopPrice)
        .def("getTimestamp", &Order::getTimestamp)
        .def("printOrder", &Order::printOrder);

    py::class_<Summary>(m, "Summary")
        .def("printSummary", &Summary::printSummary)
        .def("getBestPrice", &Summary::getBestPrice);

    py::class_<Engine>(m, "Engine")
        .def(py::init<>())
        .def("match", &Engine::match, py::return_value_policy::reference)
        .def("printBook", &Engine::printBook);
}
