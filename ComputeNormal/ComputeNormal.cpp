
#include <fstream>
#include <iostream>
#include <string>


#include <ANN/ANN.h>

#include <Eigen\Eigen>

int CountLines(char* fileName)
{
	std::ifstream fin(fileName, std::ifstream::in);
	std::string tmp;
	int count = 0;
	while (std::getline(fin, tmp, '\n'))
	{
		count++;
	}
	std::string();
	return count;
}

void GetPoints(char* fileName, ANNpointArray dataPts, int nPts, int dim)
{
	std::ifstream fin(fileName, std::ifstream::in);
	int n = 0;
	while (nPts--)
	{
		fin >> dataPts[n][0];
		fin >> dataPts[n][1];
		fin >> dataPts[n][2];
		n++;
	}
}

int main(int argc, char** argv)
{
	int dim = 3;
	int n = 10;

	ANNpointArray dataPts;
	ANNpoint queryPt;
	ANNkd_tree* kdTree;
	ANNidxArray nnIdx;
	ANNdistArray dists;


	int nPts = CountLines(argv[1]);
	std::cout << "点的个数：" << nPts << std::endl;

	dataPts = annAllocPts(nPts, dim);

	GetPoints(argv[1], dataPts, nPts, dim);

	queryPt = annAllocPt(dim);
	kdTree = new ANNkd_tree(dataPts, nPts, dim);
	nnIdx = new ANNidx[n];
	dists = new ANNdist[n];

	std::ofstream fout(argv[2]);

	for (size_t i = 0; i < nPts; i++)
	{
		queryPt = dataPts[i];
		kdTree->annkSearch(queryPt, n, nnIdx, dists, 0);

		double mean_x = 0.0;
		double mean_y = 0.0;
		double mean_z = 0.0;
		for (size_t j = 0; j < n; j++)
		{
			mean_x += dataPts[nnIdx[j]][0];
			mean_y += dataPts[nnIdx[j]][1];
			mean_z += dataPts[nnIdx[j]][2];
		}
		mean_x /= (double)n;
		mean_y /= (double)n;
		mean_z /= (double)n;
		Eigen::MatrixXd matA(dim, n);
		for (size_t j = 0; j < n; j++)
		{
			matA(0, j) = dataPts[nnIdx[j]][0] - mean_x;
			matA(1, j) = dataPts[nnIdx[j]][1] - mean_y;
			matA(2, j) = dataPts[nnIdx[j]][2] - mean_z;
		}

		//std::cout << dataPts[i][0] << ',' << dataPts[i][1] << ',' << dataPts[i][2] << std::endl;

		Eigen::Vector3d normalVector;

#ifdef SVD
		Eigen::JacobiSVD<Eigen::Matrix3d> svd(matA * matA.transpose(),
			Eigen::ComputeFullU);
		//std::cout << "Its singular values are:" << std::endl << svd.singularValues() << std::endl;
		//std::cout << "Its left singular vectors are the columns of the thin U matrix:" << std::endl << svd.matrixU() << std::endl;
		//std::cout << "Its right singular vectors are the columns of the thin V matrix:" << std::endl << svd.matrixV() << std::endl;
		normalVector = svd.matrixU().col(2);
		//std::cout << normalVector << std::endl;
#endif // SVD

#ifndef SVD
		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigenSolver(matA * matA.transpose());
		Eigen::Vector3d eigenValues = eigenSolver.eigenvalues();
		int minIndex = 0;
		if (abs(eigenValues[1]) < abs(eigenValues[minIndex]))
			minIndex = 1;
		if (abs(eigenValues[2]) < abs(eigenValues[minIndex]))
			minIndex = 2;
		normalVector = eigenSolver.eigenvectors().col(minIndex);
		normalVector.normalize();
		//std::cout << normalVector << std::endl << std::endl;;
#endif // !SVD

		fout << dataPts[i][0] << ' ' << dataPts[i][1] << ' ' << dataPts[i][2] << ' '
			<< normalVector(0) << ' ' << normalVector(1) << ' ' << normalVector(2) << std::endl;;

	}
}