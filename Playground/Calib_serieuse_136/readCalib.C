#include "../../../lib/libRoot.hpp"
#include "../../../lib/Classes/Detectors.hpp"

void readCalib(std::string const & filename = "136_Eu.calpoints")
{
  std::ifstream file(filename, std::ios::in);
  detectors.load("../index_129.list");

  std::vector<int> data_column_index = {1,2,3,4,5}; // The index of the wanted rows
  int label_index = 0;                              // The index of the row label (usually = 0)
  char delim = ';';                                 // The delimiter of the csv file

  int const & N = data_column_index.size(); // The number of wanted rows
  std::vector<std::string> m_header;        // The header still in strings
  std::vector<double> data_points;          // The value of the data points (the label of wanted columns casted in double)
  std::vector<std::vector<double>> m_data;  // The measured value
  bool m_ok = true;                         // Is everything ok ?
  // bool m_warning_is_error = false;
  bool m_warning_is_error = true;

  std::vector<std::string> labels;          // The label of the row
  
  // Handle the header : 
  std::string reader;
  std::getline(file, reader);
  m_header = getList(reader, delim);
  int const & nb_columns = m_header.size();

  // Loop over all the wanted rows
  for (int index = 0; index<N; index++) 
  {
    auto const & data_index = data_column_index[index]; // The actual index of the row in the file
    if (data_index>=nb_columns) Colib::throw_error("Column "+ std::to_string(data_index) + " out of bonds");
    try
    {
      data_points.push_back(std::stod(m_header[data_index]));
    }
    catch(const std::exception& e)
    {
      // If m_header[data_index] is not convertible to double then this column is of no interest
      std::cerr << e.what() << '\n';
      data_column_index.erase(data_column_index.begin()+index);
    }
  }

  // Handle the data :
  int row_i = 0; // Row number
  while (std::getline(file, reader))
  {
    std::vector<std::string> data = getList(reader, delim);
    // for (auto const & e : data) std::cout << "\"" << e << "\" ";
    // std::cout << "\"" << std::endl;
    if (data.size() != nb_columns)
    {
      // Handle the case where the number of types in the header
      // doesn't match the number of template arguments.
      print("Error: Number of entries at line", row_i, "do not match the header size");
      m_ok = false;
    }
    else
    {
      labels.push_back(data[label_index]);

      // The line containing the data :
      std::vector<double> newLineData(N, -1);

      // Loop over all the wanted rows :
      for (int index = 0; index<N; index++)
      {
        auto const & data_index = data_column_index[index];// The actual index of the row in the file
      //   try
      //   {
      //     newLineData[index] = std::stod(data[data_index]);
      //   }
      //   catch(const std::exception& e)
      //   {
      //     // m_data[index].push_back(-1);
      //     // if (m_warning_is_error) Colib::throw_error(e.what());
      //     print("line", row_i, "column", index, "raised", e.what());
      //   }
      // }
      // Add the line to the whole data :
      m_data.emplace_back(newLineData);
    }
    row_i++;
  }

  if (m_ok == false) return;

  int n = row_i;

  std::string outName = removeExtension(filename)+".recal";
  std::ofstream out(outName, std::ios::out);

  std::vector<std::unique_ptr<TGraphErrors>> graphs;
  std::unique_ptr<TGraph> all_residues(new TGraph());
  std::vector<std::unique_ptr<TGraph>> residues;
  for (int row = 0; row<n; row++)
  {
    print(labels[row]);
    auto const & name = labels[row];
    auto const & data = m_data[row];
    auto const & label = detectors[name];
    auto N_points = 0;
    std::vector<double> x ;
    std::vector<double> y ;
    std::vector<double> ex;
    std::vector<double> ey;
    for (int c = 0; c<N; c++) 
    {
      auto const & v = data[c];
      if (v>0)
      {
        N_points++;
        x .push_back(v);
        y .push_back(data_points[c]);
        ex.push_back(sqrt(v)/2);
        ey.push_back(0);
      }
    }
    
    if (x.size() != y.size() || N_points!=x.size()) Colib::throw_error("size !!");

    graphs.emplace_back(new TGraphErrors(N_points,x.data(),y.data(),ex.data(),ey.data()));
    auto & graph = graphs.back();
    graph -> SetName((name+"_gr").c_str());

    std::unique_ptr<TF1> linear(new TF1((name+"_lin").c_str(),"[0]*x"));
    graph->Fit(linear.get(),"Wq");

    // std::unique_ptr<TF1> quadratic(new TF1((name+"_quad").c_str(),"pol2"));
    // quadratic->SetParameter(0, linear -> GetParameter(0));
    // quadratic->SetParameter(1, linear -> GetParameter(1));
    // graph->Fit(quadratic.get(),"q");

    // out << name << " " << quadratic -> GetParameter(0) << " " << quadratic -> GetParameter(1) << std::endl;
    // out << name << " " << linear -> GetParameter(0) << " " << linear -> GetParameter(1) << std::endl;
    out << label << " " << 0 << " " << linear -> GetParameter(0) << std::endl;

    // TList *functionsList = graph->GetListOfFunctions();
    // functionsList->Remove(quadratic.get());
    // functionsList->Remove(linear.get());

    // Calculate the residues :
    residues.emplace_back(new TGraph());
    auto & residue = residues.back();
    residue -> SetName((name+"_res").c_str());
    for (size_t c = 0; c<x.size(); c++)
    {
      all_residues->AddPoint(y[c], y[c]-linear->Eval(x[c]));
      residue->AddPoint(y[c], y[c]-linear->Eval(x[c]));
    }
  }
  out.close();

  std::string outRootName = "Cal_" + removeExtension(outName) + ".root";
  auto rootFile = TFile::Open(outRootName.c_str(), "RECREATE");
  rootFile->cd();

  all_residues->SetName("Residues");
  all_residues->SetMarkerStyle(5);
  all_residues->SetMarkerSize(1);
  all_residues->SetMarkerColor(kRed);
  all_residues->Write();

  for (auto & graph : graphs) 
  {
    // auto histo = new TH1D(graph->GetName(), graph->GetName(), graph->GetN(), graph->GetX()[0], graph->GetX()[graph->GetN()-1]);
    // histo->SetDirectory(nullptr);
    // for (int c = 0; c < graph->GetN(); c++) histo->Fill(graph->GetX()[c], graph->GetY()[c]);
    // histo->Write();
    // auto c = new TCanvas(graph->GetName(), graph->GetName());
    // c->cd();
    graph->SetMarkerStyle(5);
    graph->SetMarkerSize(1);
    graph->SetMarkerColor(kRed);
    // graph->Draw();
    graph->Write();
    // c->Write();
  }  

  for (auto & residue : residues)
  {
    residue->SetMarkerStyle(8);
    residue->SetMarkerSize(1);
    residue->SetMarkerColor(kRed);
    residue->Write();
  }

  rootFile->Write();
  rootFile->Close();

  print(outRootName, "written");
}