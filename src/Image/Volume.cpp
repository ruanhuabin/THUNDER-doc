/*******************************************************************************
 * Author: Mingxu Hu
 * Dependency:
 * Test:
 * Execution:
 * Description:
 *
 * Manual:
 * ****************************************************************************/

#include "Volume.h"

Volume::Volume() {}

Volume::Volume(const int nCol,
               const int nRow,
               const int nSlc,
               const int space)
{
    alloc(nCol, nRow, nSlc, space);
}

Volume::~Volume() {}

void Volume::alloc(int space)
{
    alloc(_nCol, _nRow, _nSlc, space);
}

void Volume::alloc(const int nCol,
                   const int nRow,
                   const int nSlc,
                   const int space)
{
    _nCol = nCol;
    _nRow = nRow;
    _nSlc = nSlc;

    if (space == RL_SPACE)
    {
        clearRL();

        _sizeRL = nCol * nRow * nSlc;
        _sizeFT = (nCol / 2 + 1) * nRow * nSlc;

        _dataRL.reset(new double[_sizeRL]);
    }
    else if (space == FT_SPACE)
    {
        clearFT();

        _sizeRL = nCol * nRow * nSlc;
        _sizeFT = (nCol / 2 + 1) * nRow * nSlc;

        _dataFT.reset(new Complex[_sizeFT]);
    }
}

double Volume::getRL(const int iCol,
                     const int iRow,
                     const int iSlc) const
{
    // coordinatesInBoundaryRL(iCol, iRow, iSlc);
    return _dataRL[VOLUME_INDEX_RL((iCol >= 0) ? iCol : iCol + _nCol,
                                   (iRow >= 0) ? iRow : iRow + _nRow,
                                   (iSlc >= 0) ? iSlc : iSlc + _nSlc)];
}

void Volume::setRL(const double value,
                   const int iCol,
                   const int iRow,
                   const int iSlc)
{
    // coordinatesInBoundaryRL(iCol, iRow, iSlc);

    _dataRL[VOLUME_INDEX_RL((iCol >= 0) ? iCol : iCol + _nCol,
                            (iRow >= 0) ? iRow : iRow + _nRow,
                            (iSlc >= 0) ? iSlc : iSlc + _nSlc)] = value;
}

void Volume::addRL(const double value,
                   const int iCol,
                   const int iRow,
                   const int iSlc)
{
    // coordinatesInBoundaryRL(iCol, iRow, iSlc);

    #pragma omp atomic
    _dataRL[VOLUME_INDEX_RL((iCol >= 0) ? iCol : iCol + _nCol,
                            (iRow >= 0) ? iRow : iRow + _nRow,
                            (iSlc >= 0) ? iSlc : iSlc + _nSlc)] += value;
}

Complex Volume::getFT(int iCol,
                      int iRow,
                      int iSlc,
                      const ConjugateFlag cf) const
{
    // coordinatesInBoundaryFT(iCol, iRow, iSlc);
    bool flag;
    size_t index;
    VOLUME_FREQ_TO_STORE_INDEX(index, flag, iCol, iRow, iSlc, cf);
    return flag ? CONJUGATE(_dataFT[index]) : _dataFT[index];
}

void Volume::setFT(const Complex value,
                   int iCol,
                   int iRow,
                   int iSlc,
                   const ConjugateFlag cf)
{
    // coordinatesInBoundaryFT(iCol, iRow, iSlc);

    bool flag;
    size_t index;
    VOLUME_FREQ_TO_STORE_INDEX(index, flag, iCol, iRow, iSlc, cf);

    _dataFT[index] = flag ? CONJUGATE(value) : value;
}

void Volume::addFT(const Complex value,
                   int iCol,
                   int iRow,
                   int iSlc,
                   const ConjugateFlag cf)
{
    // coordinatesInBoundaryFT(iCol, iRow, iSlc);
    bool flag;
    size_t index;
    VOLUME_FREQ_TO_STORE_INDEX(index, flag, iCol, iRow, iSlc, cf);

    Complex val = flag ? CONJUGATE(value) : value;

    #pragma omp atomic
    _dataFT[index].dat[0] += val.dat[0];
    #pragma omp atomic
    _dataFT[index].dat[1] += val.dat[1];
}

double Volume::getByInterpolationRL(const double iCol,
                                    const double iRow,
                                    const double iSlc,
                                    const int interp) const
{
    double w[2][2][2];
    int x0[3];
    double x[3] = {iCol, iRow, iSlc};
    switch (interp)
    {
        case NEAREST_INTERP: WG_TRI_NEAREST(w, x0, x); break;
        case LINEAR_INTERP: WG_TRI_LINEAR(w, x0, x); break;
        case SINC_INTERP: WG_TRI_SINC(w, x0, x); break;
    }

    return getRL(w, x0);
}

Complex Volume::getByInterpolationFT(double iCol,
                                     double iRow,
                                     double iSlc,
                                     const int interp) const
{
    bool cf = VOLUME_CONJUGATE_HALF(iCol, iRow, iSlc);

    double w[2][2][2];
    int x0[3];
    double x[3] = {iCol, iRow, iSlc};
    switch (interp)
    {
        case NEAREST_INTERP: WG_TRI_NEAREST(w, x0, x); break;
        case LINEAR_INTERP: WG_TRI_LINEAR(w, x0, x); break;
        case SINC_INTERP: WG_TRI_SINC(w, x0, x); break;
    }

    Complex result = getFT(w, x0, conjugateNo);
    return cf ? CONJUGATE(result) : result;
}

void Volume::addFT(const Complex value,
                   const double iCol,
                   const double iRow,
                   const double iSlc,
                   const double a,
                   const double alpha)
{
    VOLUME_SUB_SPHERE_FT(a)
    {
        double r = NORM_3(iCol - i, iRow - j, iSlc - k);
        if (r < a) addFT(value * MKB_FT(r, a, alpha), i, j, k);
    }
}

void Volume::addFT(const Complex value,
                   const double iCol,
                   const double iRow,
                   const double iSlc,
                   const double a,
                   const TabFunction& kernel)
{
    VOLUME_SUB_SPHERE_FT(a)
    {
        double r = NORM_3(iCol - i, iRow - j, iSlc - k);
        if (r < a) addFT(value * kernel(r), i, j, k);
    }
}

void Volume::clear()
{
    ImageBase::clear();

    _nCol = 0;
    _nRow = 0;
    _nSlc = 0;
}

Volume Volume::copyVolume() const
{
    Volume out;

    copyBase(out);
    out._nCol = _nCol;
    out._nRow = _nRow;
    out._nSlc = _nSlc;

    return out;
}

void Volume::coordinatesInBoundaryRL(const int iCol,
                                     const int iRow,
                                     const int iSlc) const
{
    if ((iCol < -_nCol / 2) || (iCol >= _nCol / 2) ||
        (iRow < -_nRow / 2) || (iRow >= _nRow / 2) ||
        (iSlc < -_nSlc / 2) || (iSlc >= _nSlc / 2))
        CLOG(FATAL, "LOGGER_SYS") << "Accessing Value out of Boundary";
}

void Volume::coordinatesInBoundaryFT(const int iCol,
                                     const int iRow,
                                     const int iSlc) const
{
    if ((iCol < -_nCol / 2) || (iCol > _nCol / 2) ||
        (iRow < -_nRow / 2) || (iRow >= _nRow / 2) ||
        (iSlc < -_nSlc / 2) || (iSlc >= _nSlc / 2))
        CLOG(FATAL, "LOGGER_SYS") << "Accessing Value out of Boundary";
}

double Volume::getRL(const double w[2][2][2],
                     const int x0[3]) const
{
    double result = 0;
    FOR_CELL_DIM_3 result += getRL(x0[0] + i, x0[1] + j, x0[2] + k)
                           * w[i][j][k];
    return result;
}

Complex Volume::getFT(const double w[2][2][2],
                      const int x0[3],
                      const ConjugateFlag conjugateFlag) const
{
    Complex result = COMPLEX(0, 0);
    FOR_CELL_DIM_3 result += getFT(x0[0] + i,
                                   x0[1] + j,
                                   x0[2] + k,
                                   conjugateFlag)
                           * w[i][j][k];
    return result;
}
