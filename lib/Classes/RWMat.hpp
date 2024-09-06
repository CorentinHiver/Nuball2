#ifndef RWMAT_HPP
#define RWMAT_HPP

#ifndef LIBROOT_HPP
  #include <string>
  #include <TH2.h>
  #include <TH2F.h>
  #include <TH3I.h>
  #include <iostream>
  #include <iomanip>
  #include <vector>

  ////////////////////////////////////
  // USER FRIENDLY PRINT FUNCTION : //
  ////////////////////////////////////

  void print() {std::cout << std::endl;}

  template <class T>
  std::ostream& operator<<(std::ostream& cout, std::vector<T> const & v)
  {
    for (auto const & e : v) cout << e << " ";
    return cout;
  }

  /// @brief Generic print
  /// @details Automatically adds space between each input. Terminate the output with a "\\n"
  template <class T> 
  void print(T const & t) {std::cout << t << std::endl;}

  /// @brief Generic print
  /// @details Automatically adds space between each input. Terminate the output with a "\\n"
  template <class T, class... T2> 
  void print(T const & t, T2 const &... t2) {std::cout << t << " "; print(t2...);}
#endif //LIBROOT_HPP

////////////
// CODE : //
////////////

/**
 * @brief From Jon
 */
class RWMat
{
 public:
  RWMat(std::string name="test.m4b",int nchans=4096); //@- Default constructor
  RWMat(TH2D* RootMat);  //@- constructor from Root 2D histogram
  RWMat(TH2F* RootMat);  //@- constructor from Root 2D histogram
  RWMat(TH2I* RootMat);  //@- constructor from Root 2D histogram
  #ifdef MTTHIST_HPP
  template<class T>
  RWMat(MultiHist<T> & MTRootMat);  //@- constructor from MTThist 2D histogram
  #endif //MTTHIST_HPP
  ~RWMat();       //@- Normal destructor
  template<class THist>
  void Reset(THist* RootMat);
  void Write(std::string name="", std::string path = "./"); //write the RWMat out
  void Read(std::string Filename="", bool IsInteger=true); //Read matrix from file
  double Get(unsigned short i, unsigned short j) {return fRWMat[i][j];}
  auto const & GetName () const noexcept {return fName;}
  void GetName (std::string name) noexcept {fName = name;}
  void Set(unsigned short i, unsigned short j, int val) {fRWMat[i][j]=val;}
  void Set(unsigned short i, unsigned short j, double val) {fRWMat[i][j]=val;}
  void Fill(unsigned short i, unsigned short j); //increment channel by 1 count
  TH2F* RootHisto()
  {
    TH2F* ret = new TH2F(fName.c_str(), fName.c_str(), 4096,0,4096, 4096,0,4096);
    for (int i = 0; i<4096; ++i) for (int j = 0; j<4096; ++j)
    {
      ret->SetBinContent(i, j, this->Get(i, j));
    }
    return ret;
  }
  TH1F* Proj(ushort i)
  {
    TH1F* proj = new TH1F((fName+"_p"+std::to_string(i)).c_str(), (fName+"_p"+std::to_string(i)).c_str(), 4096,0,4096);
    for (ushort bin = 0; bin<4096; ++bin) proj->SetBinContent(bin, this->Get(i, bin));
    return proj;
  }
  TH1F* Proj(ushort imin, ushort imax)
  {
    TH1F* proj = new TH1F((fName+"_p"+std::to_string(imin)+"_"+std::to_string(imax)).c_str(), (fName+"_p"+std::to_string(imin)+"_"+std::to_string(imax)).c_str()
    , 4096,0,4096);
    for (ushort i = imin; i<imax; ++i) for (ushort bin = 0; bin<4096; ++bin) proj->SetBinContent(bin, fRWMat[i][bin]);
    return proj;
  }
  RWMat* Add(RWMat* Matrix,double val);
  double Integral();
  void ReSymmetrise();
  double FindMinMax();
  int FindMinChan();

protected:
  unsigned short   fNChannels = 0;
  std::string     fName;
  int**  fRWMat = nullptr;
  double    fTotalCounts = 0.0;
  double    fMaxCounts = 0.0;
  bool m_ok = true;
  bool deleted = false;
};

RWMat::RWMat(std::string name, int nchans) //default constructor
{
  fNChannels=nchans;
  fName=name;
  fRWMat=new int*[fNChannels];
  for(int i=0 ; i < fNChannels ; i++) fRWMat[i] = new int[fNChannels];
}

#ifdef MTTHIST_HPP
template<class T>
RWMat::RWMat(MultiHist<T> & MTRootMat)
{
  if (MTRootMat.Integral() < 1)
  {
    m_ok = false;
    return;
  }
  MTRootMat.Merge();
  if (!MTRootMat -> InheritsFrom("TH2")) print(MTRootMat.GetName(),"is not a TH2 !!");
  else Reset(MTRootMat.get());
}
#endif //MTTHIST_HPP

RWMat::RWMat(TH2F* RootMat) //constructor from root object
{
  Reset(RootMat);
}

RWMat::RWMat(TH2I* RootMat) //constructor from root object
{
  Reset(RootMat);
}

RWMat::RWMat(TH2D* RootMat) //constructor from root object
{
  Reset(RootMat);
}

template<class THist>
void RWMat::Reset(THist* RootMat)
{
  if (RootMat->Integral() < 0)
  {
    print(RootMat->GetName(), "empty !!");
    m_ok = false;
    return;
  }
  if (!RootMat -> InheritsFrom("TH2")) {print(RootMat->GetName(),"is not a TH2 !!"); m_ok = false; return;}
  int xchans=RootMat->GetNbinsX();
  int ychans=RootMat->GetNbinsY();
  if (xchans != ychans) {print(RootMat->GetName(),"is not square !!"); m_ok = false; return;}
  if (xchans != 4096 || ychans != 4096) {print(RootMat->GetName(),"needs to be a 4096x4096 matrix!"); m_ok = false; return;}
  fNChannels=4096;
  fName=RootMat->GetName();
  fName+=".m4b";
  print(fName);
  fRWMat=new int*[fNChannels];
  for(int i=0 ; i < fNChannels ; i++) fRWMat[i] = new int[fNChannels];
  double val=0;
  for (int i=0; i < xchans ; i++) for (int j=0; j < ychans ; j++)
  {
    fRWMat[i][j]=RootMat->GetBinContent(i,j);
    val+=fRWMat[i][j];
  }
}
//________________________________________________________________________
RWMat::~RWMat()
{
  if (deleted)
  {
    print("RWMat", fName, "double delete, be careful !!");
  }
  else if (m_ok)
  {
    for (int i=0; i < fNChannels; i++) delete [] fRWMat[i];
    delete [] fRWMat;
    deleted = true;
  }
}
//________________________________________________________________________
void RWMat::Fill(unsigned short i, unsigned short j)
{
  if ((i < fNChannels) && (j < fNChannels)){fRWMat[i][j]++;}
}
//________________________________________________________________________
void RWMat::Read(std::string fname, bool IsInteger)
{
  fName=fname;
  FILE *fprad;

  fprad = fopen(fname.c_str(),"r");
  if (fprad) {std::cout << "Reading RWMat : " << fname << std::endl;}
  else {std::cout << "Error Reading RWMat : " << fname << std::endl; exit(1);}

  int size=fNChannels;

  double* buffer=new  double[size];
  int* bufferi=new  int[size];

  std::cout << "Number of channels = " << fNChannels <<  std::endl;
  for (int i=0; i<size; i++)
  {
    fread(bufferi, size, ( (IsInteger) ? sizeof(int) : sizeof(double) ), fprad);
    for (int j=0; j<size; j++) fRWMat[i][j]=bufferi[j];
  }
  fclose(fprad);
  delete [] buffer;
  delete [] bufferi;
}
  //________________________________________________________________________
void RWMat::Write(std::string name, std::string path)
{
  if (!m_ok) {print("CAN'T WRITE", name, "RADWARE MATRIX"); return;}
  FILE *fprad;
  if (name=="") {name=fName;}
  else {fName=name;}
  if (path.back() != '/') path.push_back('/');
  name = path+name;
  fprad = fopen(name.c_str(),"w");
  if (fprad) {std::cout << "Writing RWMat : " << name << std::endl;}
  else {std::cout << "Error Writing RWMat : " << name << std::endl; exit(1);}

  int size=fNChannels;

  int* buffer=new  int[size];

  std::cout << "channels = " << fNChannels << " counts = " << this->Integral() << " size = " << size*size*sizeof(int)/1048576 << "Mo" << std::endl;

  for (int i=0; i<size; i++)
  {
    for (int j=0; j<size; j++)
    {
      buffer[j]=fRWMat[i][j];
    }
    fwrite(buffer, size, sizeof(int), fprad);
  }

  fclose(fprad);
  delete [] buffer;
}
//________________________________________________________________________
RWMat* RWMat::Add(RWMat* Matrix, double val)
{
  RWMat *mat3=new RWMat();
  for (int i=0; i<fNChannels; i++)
    for (int j=0; j<fNChannels; j++)
      mat3->Set(i,j,(fRWMat[i][j]+(Matrix->Get(i,j)*val)));
  return mat3;
}
//________________________________________________________________________
double RWMat::Integral()
{
  double val=0.;
  for (int i=0; i<fNChannels; i++) for (int j=0; j<fNChannels; j++) val+=fRWMat[i][j];
  return val;
}

//________________________________________________________________________
void RWMat::ReSymmetrise()
{
  for (int i=0; i<fNChannels; i++)
  {
    for (int j=0; j<i; j++)
    {
      double val1=fRWMat[i][j];
      double val2=fRWMat[j][i];
      fRWMat[i][j]=(val1+val2)/2.0;
      fRWMat[j][i]=(val1+val2)/2.0;
    }
  }
}
//________________________________________________________________________
double RWMat::FindMinMax()
{
  double maxval=0;
  double minval=0;
  int maxx = 0, maxy = 0, minx = 0, miny = 0;

  for (int i=0; i<fNChannels; i++)
  {
    for (int j=0; j<i; j++)
    {
      double val1=fRWMat[i][j];
      if (val1 > maxval) {maxval=val1; maxx=i; maxy=j;}
      if (val1 < minval) {minval=val1; minx=i; miny=j;}
    }
  }
  std::cout << "Max Value = " << maxval << " at " <<maxx << " "<<maxy<<std::endl;
  std::cout << "Min Value = " << minval << " at " <<minx << " "<<miny<<std::endl;
  return minval;
}
//________________________________________________________________________
int RWMat::FindMinChan()
{
  double minval=0;
  int minx = 0, miny = 0;

  for (int i=0; i<fNChannels; i++)
  {
    for (int j=0; j<i; j++)
    {
      double val1=fRWMat[i][j];
      if (val1 < minval) {minval=val1; minx=i; miny=j;}
    }
  }
  if (minx > miny) {miny=minx;}
  return miny;
}
#endif //RWMAT_HPP