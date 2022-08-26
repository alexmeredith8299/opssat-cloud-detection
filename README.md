# OPS-SAT Cloud Detection
An [OPS-SAT](https://opssat1.esoc.esa.int/) experiment with image processing algorithms developed for BeaverCube-2, a project under development between the [MIT Space Telecommunications, Astronomy, Radiation (STAR) Lab](https://starlab.mit.edu/) and the Northrop Grumman Corporation. The algorithms were uploaded to and executed on OPS-SAT, a 3U CubeSat owned and operated by [ESA](https://www.esa.int/) with a processing payload that allows rapid prototyping, testing, and validation of software and firmware experiments in space at no cost to the experimenter. Testing these algorithms onboard OPS-SAT significantly reduces risk for future on-orbit image processing missions such as BeaverCube-2. We focus on four image processing algorithms used for cloud detection: a luminosity-thresholding method, a random forest method, an U-Net based deep learning method — all developed by STAR Lab for BeaverCube-2 — and a k-means clustering deep learning method implemented by the OPS-SAT Flight Control Team (FCT). We evaluate each method in terms of in terms of overall accuracy, power draw, and temperature rise on-orbit, and discuss the challenges of implementing these methods on embedded hardware and the lessons learned for BeaverCube-2.

Read more about this experiment in **Kacker, S., Meredith, A., Cahoy, K., & Labrèche, G. (2022). Machine Learning Image Processing Algorithms Onboard OPS-SAT.**
https://digitalcommons.usu.edu/smallsat/2022/all2022/65/ 

```bibtex
@article{kacker2022machine,
  title={Machine Learning Image Processing Algorithms Onboard OPS-SAT},
  author={Kacker, Shreeyam and Meredith, Alex and Cahoy, Kerri and Labr{\`e}che, Georges},
  year={2022}
}
```

## Build with CMake

1. Navigate to `bin`
2. Generate the CMake files with `cmake ../`
3. Once the makefiles are generated, build with `cmake --build .`

## Run random forest EM test

1. Build binaries (see "Build with CMake" section)
2. Run `bash random_forest.sh` (or just run `random_forest.sh` if using bash as your shell)
