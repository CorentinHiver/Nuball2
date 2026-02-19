#ifndef PARISLABEL_H
#define PARISLABEL_H

class ParisLabel
{
public:
  ParisLabel(){};
  ParisLabel(Label const & l) {setLabel(l);}
  void setLabel(Label const & l)
  {
    back = back_array[l];
    ring = ring_array[l];
    label = label_array[l];
  }

  static void setArrays();
  Bool_t back   = false; //true : back ; false front
  uchar  ring   = 0;
  uchar  label  = 0;

private:
  static std::vector<Double_t> paris_labels;
  static LabelsArray<bool> back_array;
  static LabelsArray<uchar> ring_array;
  static LabelsArray<uchar> label_array;
  static bool arrays_set;
};

LabelsArray<bool> ParisLabel::back_array;
LabelsArray<uchar> ParisLabel::ring_array;
LabelsArray<uchar> ParisLabel::label_array;

void ParisLabel::setArrays()
{
  for (size_t l = 0; l<1000; l++)
  {
         if (l<300) { back_array[l] = true;  ring_array[l] = 1; label_array[l] = l-200; }
    else if (l<400) { back_array[l] = true;  ring_array[l] = 2; label_array[l] = l-300; }
    else if (l<500) { back_array[l] = true;  ring_array[l] = 3; label_array[l] = l-400; }
    else if (l<600) { back_array[l] = false; ring_array[l] = 1; label_array[l] = l-500; }
    else if (l<700) { back_array[l] = false; ring_array[l] = 2; label_array[l] = l-600; }
    else if (l<800) { back_array[l] = false; ring_array[l] = 3; label_array[l] = l-700; }
  }
}

bool ParisLabel::arrays_set = false;

std::vector<Double_t> paris_labels =
{
  201, 202, 203, 204, 205, 206, 207, 208,
  301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316,
  401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412,
  501, 502, 503, 504, 505, 506, 507, 508,
  601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616,
  701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712
};

#endif //PARISLABEL_H
