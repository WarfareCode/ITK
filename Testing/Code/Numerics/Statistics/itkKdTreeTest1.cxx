/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkKdTreeTest1.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkVector.h"
#include "itkMersenneTwisterRandomVariateGenerator.h"
#include "itkListSample.h"
#include "itkKdTree.h"
#include "itkKdTreeGenerator.h"
#include "itkEuclideanDistance.h"

int itkKdTreeTest1(int , char * [] )
{
  // Random number generator
  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator NumberGeneratorType;

  NumberGeneratorType::Pointer randomNumberGenerator = NumberGeneratorType::New();
  randomNumberGenerator->Initialize();
  
  typedef itk::Array< double > MeasurementVectorType ;
  typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType ;

  const SampleType::MeasurementVectorSizeType measurementVectorSize = 2;

  SampleType::Pointer sample = SampleType::New() ;
  sample->SetMeasurementVectorSize( measurementVectorSize );

  // 
  // Generate a sample of random points
  //
  MeasurementVectorType mv( measurementVectorSize ) ;
  for (unsigned int i = 0 ; i < 1000 ; ++i )
    {
    mv[0] = randomNumberGenerator->GetNormalVariate( 0.0, 1.0 );
    mv[1] = randomNumberGenerator->GetNormalVariate( 0.0, 1.0 );
    sample->PushBack( mv ) ;
    }

  typedef itk::Statistics::KdTreeGenerator< SampleType > TreeGeneratorType ;
  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New() ;

  treeGenerator->SetSample( sample ) ;
  treeGenerator->SetBucketSize( 16 ) ;
  treeGenerator->Update() ;

  typedef TreeGeneratorType::KdTreeType TreeType ;
  typedef TreeType::NearestNeighbors NeighborsType ;
  typedef TreeType::KdTreeNodeType NodeType ;

  TreeType::Pointer tree = treeGenerator->GetOutput() ;

  MeasurementVectorType queryPoint( measurementVectorSize ) ;

  unsigned int numberOfNeighbors = 1 ;
  TreeType::InstanceIdentifierVectorType neighbors ;

  MeasurementVectorType result( measurementVectorSize ) ;
  MeasurementVectorType test_point( measurementVectorSize ) ;
  MeasurementVectorType min_point( measurementVectorSize ) ;

  unsigned int numberOfFailedPoints = 0;

  const unsigned int numberOfTestPoints = 1000;

  //
  // Generate a second sample of random points
  // and use them to query the tree
  //
  for (unsigned int i = 0 ; i < numberOfTestPoints ; ++i )
    {

    double min_dist = itk::NumericTraits< double >::max();

    queryPoint[0] = randomNumberGenerator->GetNormalVariate( 0.0, 1.0 );
    queryPoint[1] = randomNumberGenerator->GetNormalVariate( 0.0, 1.0 );

    tree->Search( queryPoint, numberOfNeighbors, neighbors ) ;

    //
    // The first neighbor should be the closest point.
    //
    result = tree->GetMeasurementVector( neighbors[0] );

    // 
    // Compute the distance to the "presumed" nearest neighbor
    //
    double result_dist = sqrt(
          (result[0] - queryPoint[0]) *
          (result[0] - queryPoint[0]) +
          (result[1] - queryPoint[1]) *
          (result[1] - queryPoint[1])
          );

    //
    // Compute the distance to all other points, to verify
    // whether the first neighbor was the closest one or not.
    //
    for( unsigned int i = 0 ; i < 1000 ; ++i )
      {
      test_point = tree->GetMeasurementVector( i );

      const double dist = sqrt(
          (test_point[0] - queryPoint[0]) *
          (test_point[0] - queryPoint[0]) +
          (test_point[1] - queryPoint[1]) *
          (test_point[1] - queryPoint[1])
          );

      if( dist < min_dist )
        {
        min_dist = dist;
        min_point = test_point;
        }
      }

    if( min_dist < result_dist )
      {
      std::cerr << "Problem found " << std::endl;
      std::cerr << "Query point " << queryPoint << std::endl;
      std::cerr << "Reported closest point " << result
                << " distance " << result_dist << std::endl;
      std::cerr << "Actual   closest point " << min_point
                << " distance " << min_dist << std::endl;
      std::cerr << std::endl;
      std::cerr << "Test FAILED." << std::endl;
      numberOfFailedPoints++;
      }

    }
  
  if( numberOfFailedPoints )
    {
    std::cerr << numberOfFailedPoints << " failed out of " 
              << numberOfTestPoints << " points " << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Test PASSED." << std::endl;
  return EXIT_SUCCESS;
}
