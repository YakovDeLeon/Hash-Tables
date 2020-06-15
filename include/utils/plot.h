#ifndef _PLOT_
#define _PLOT_
#include<matplotlibcpp.h>
#include<iostream>
#include<filesystem>

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void print_test(const std::string &test_name)
{
    std::cout << "==============================================================================================================\n";
    std::cout << test_name + "\n";
    std::cout << "==============================================================================================================\n";
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void plot_settings(size_t w, size_t h, 
        const std::string &x_label, const std::string &y_label,
        const std::string &title,  bool need_grid = true)
{
    namespace plt = matplotlibcpp;
    plt::figure_size(w, h);
    plt::xlabel(x_label);
    plt::ylabel(y_label);
    plt::grid(need_grid);
    plt::title(title);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void save_plot(const std::string &dir_name, const std::string &plot_name,
                         bool need_legend = true)
{
    namespace plt = matplotlibcpp;
    std::filesystem::create_directory(dir_name);
    if (need_legend) plt::legend();
    plt::save(dir_name + plot_name);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
inline void plot_graph(const std::vector<Val> &x,
                       const std::vector<Val> &y,
                       const std::string &line_name,
                       const std::string &color_line = "")
{
    namespace plt = matplotlibcpp;
    plt::named_plot(line_name, x, y, color_line);
//plt::scatter(x, y, 50, { { "c", "red"  }, { "marker", "."  } });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
inline void plot_scatter(const std::vector<Val> &x,
                       const std::vector<Val> &y,
                       double point_size,
                       const std::string &color_line = "")
{
    namespace plt = matplotlibcpp;
    plt::scatter(x, y, point_size, {{ "c", color_line  }, { "marker", "."  }});
//plt::scatter(x, y, 50, { { "c", "red"  }, { "marker", "."  } });
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
