/*******************************************************************************
 * Author: Mingxu Hu
 * Dependency:
 * Test:
 * Execution:
 * Description:
 *
 * Manual:
 * ****************************************************************************/

#include "Symmetry.h"

Symmetry::Symmetry() {}

Symmetry::Symmetry(const char sym[])
{
    init(sym);
}

Symmetry::Symmetry(const int pgGroup,
                   const int pgOrder)
{
    _pgGroup = pgGroup;
    _pgOrder = pgOrder;

    init();
}

Symmetry::Symmetry(const Symmetry& that)
{
    *this = that;
}

Symmetry::~Symmetry()
{
    clear();
}

Symmetry& Symmetry::operator=(const Symmetry& that)
{
    clear();

    _pgGroup = that.pgGroup();
    _pgOrder = that.pgOrder();

    mat33 L, R;
    for (size_t i = 0; i < that.nSymmetryElement(); i++)
    {
        that.get(L, R, i);
        append(L, R);
    }

    return *this;
}

void Symmetry::init(const char sym[])
{
    symmetryGroup(_pgGroup, _pgOrder, sym);

    init();
}

int Symmetry::pgGroup() const
{
    return _pgGroup;
}

int Symmetry::pgOrder() const
{
    return _pgOrder;
}

void Symmetry::get(mat33& L,
                   mat33& R,
                   const int i) const
{
    L = _L[i];
    R = _R[i];
}

int Symmetry::nSymmetryElement() const
{
    return _L.size();
}

void Symmetry::clear()
{
    _L.clear();
    _R.clear();
}

void Symmetry::init()
{
    vector<SymmetryOperation> entry;
    fillSymmetryEntry(entry, _pgGroup, _pgOrder);

    init(entry);
}

void Symmetry::init(const vector<SymmetryOperation>& entry)
{
    _L.clear();
    _R.clear();
    
    fillLR(entry);
    completePointGroup();
}

void Symmetry::append(const mat33& L,
                      const mat33& R)
{
    _L.push_back(L);
    _R.push_back(R);
}

void Symmetry::set(const mat33& L,
                   const mat33& R,
                   const int i)
{
    _L[i] = L;
    _R[i] = R;
}

void Symmetry::fillLR(const vector<SymmetryOperation>& entry)
{
    mat33 L, R;

    for (size_t i = 0; i < entry.size(); i++)
    {
        L.eye();

        if (entry[i].id == 0)
        {
            // rotation
            double angle = 2 * M_PI / entry[i].fold;
            // printf("angle = %f\n", angle);
            for (int j = 1; j < entry[i].fold; j++)
            {
                rotate3D(R, angle * j, entry[i].axisPlane);
                append(L, R);
            }
        }
        else if (entry[i].id == 1)
        {
            // reflexion
            reflect3D(R, entry[i].axisPlane);
            append(L, R);
        }
        else if (entry[i].id == 2)
        {
            /* inversion
             * L -> [ 1  0  0]
             *      [ 0  1  0]
             *      [ 0  0 -1]
             * R -> [-1  0  0]
             *      [ 0 -1  0]
             *      [ 0  0 -1] */
            L(2, 2) = -1;
            R.zeros();
            R.diag() = vec({-1, -1, -1});
            append(L, R);
        }
    }
}

bool Symmetry::novo(const mat33& L,
                    const mat33& R) const
{
    // check whether L and R are both identity matrix or not
    mat33 I(fill::eye);
    if (SAME_MATRIX(L, I) && SAME_MATRIX(R, I))
        return false;

    // check whether (L, R) exists in (_L, _R) or not
    for (int i = 0; i < _L.size(); i++)
        if (SAME_MATRIX(L, _L[i]) && SAME_MATRIX(R, _R[i]))
                return false;

    return true;
}

void Symmetry::completePointGroup()
{
    umat table(nSymmetryElement(),
               nSymmetryElement(),
               fill::zeros);

    int i, j;
    while ([&]
           {
                for (int row = 0; row < table.n_rows; row++)
                    for (int col = 0; col < table.n_cols; col++)
                        if (table(row, col) == 0)
                        {
                            i = row;
                            j = col;
                            table(row, col) = 1;
                    
                            return true;
                        }
                return false;
           }())

    {
        mat33 L = _L[i] * _L[j];
        mat33 R = _R[i] * _R[j];

        if (novo(L, R))
        {
            append(L, R);
            table.resize(table.n_rows + 1,
                         table.n_cols + 1);
        }
    }
}

void display(const Symmetry& sym)
{
    mat33 L, R;

    for (int i = 0; i < sym.nSymmetryElement(); i++)
    {
        printf("%03d Symmetry Element:\n", i);

        sym.get(L, R, i);

        L.print("L matrix");
        R.print("R matrix");
    }
}

bool asymmetryUnit(const vec3 dir,
                   const Symmetry& sym)
{
    double phi, theta;
    angle(phi, theta, dir);
    return asymmetryUnit(phi, theta, sym);
}

bool asymmetryUnit(const double phi,
                   const double theta,
                   const Symmetry& sym)
{
    return asymmetryUnit(phi, theta, sym.pgGroup(), sym.pgOrder());
}

bool asymmetryUnit(const double phi,
                   const double theta,
                   const int pgGroup,
                   const int pgOrder)
{
    switch (pgGroup)
    {
        case PG_CN:
            return (phi <= 2 * M_PI / pgOrder);
        
        case PG_CI:
        case PG_CS:
            return (theta <= M_PI / 2);

        case PG_CNV:
            return (phi <= M_PI / pgOrder);

        case PG_CNH:
            return ((phi <= 2 * M_PI / pgOrder) &&
                    (theta <= M_PI / 2));
        
        case PG_SN:
            return ((phi <= 4 * M_PI / pgOrder) &&
                    (theta <= M_PI / 2));

        case PG_DN:
            return ((phi >= M_PI / 2) &&
                    (phi <= 2 * M_PI / pgOrder + M_PI / 2) &&
                    (theta <= M_PI / 2));

        case PG_DNV:
        case PG_DNH:
            return ((phi >= M_PI / 2) &&
                    (phi <= M_PI / pgOrder + M_PI / 2) &&
                    (theta <= M_PI / 2));

        case PG_T:
            return ASY(T, phi, theta);

        case PG_TD:
            return ASY(TD, phi, theta);

        case PG_TH:
            return ASY(TH, phi, theta);

        case PG_O:
            return ((phi >= M_PI / 4) &&
                    (phi <= 3 * M_PI / 4) &&
                    (theta <= M_PI / 2) &&
                    ASY(O, phi, theta));

        case PG_OH:
            return ((phi >= 3 * M_PI / 2) &&
                    (phi <= 7 * M_PI / 4) &&
                    (theta <= M_PI / 2) &&
                    ASY(O, phi, theta));

        case PG_I:
        case PG_I2:
        case PG_I1:
        case PG_I3:
        case PG_I4:
        case PG_I5:
        case PG_IH:
        case PG_I2H:
        case PG_I1H:
        case PG_I3H:
        case PG_I4H:
        case PG_I5H:
            REPORT_ERROR("Point Group has not been implemented.");
    }
    __builtin_unreachable();
}

void symmetryCounterpart(double& phi,
                         double& theta,
                         const Symmetry& sym)
{
    vec3 dir;
    direction(dir, phi, theta);
    symmetryCounterpart(dir(0), dir(1), dir(2), sym);
    angle(phi, theta, dir);
}

void symmetryCounterpart(double& ex,
                         double& ey,
                         double& ez,
                         const Symmetry& sym)
{
    vec3 dir = {ex, ey, ez};
    if (asymmetryUnit(dir, sym)) return;

    mat33 L, R;
    vec3 newDir;
    for (int i = 0; i < sym.nSymmetryElement(); i++)
    {
        sym.get(L, R, i);
        newDir = R * dir;
        if (asymmetryUnit(newDir, sym))
        {
            ex = newDir(0);
            ey = newDir(1);
            ez = newDir(2);
        }
    }
}
