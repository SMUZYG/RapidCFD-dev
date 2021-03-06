/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "coupledFvPatchField.H"
#include "lduAddressingFunctors.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    fvPatchField<Type>(p, iF)
{}


template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const gpuField<Type>& f
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    fvPatchField<Type>(p, iF, f)
{}

template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const Field<Type>& f
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    fvPatchField<Type>(p, iF, f)
{}


template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const coupledFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    fvPatchField<Type>(ptf, p, iF, mapper)
{}


template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    fvPatchField<Type>(p, iF, dict)
{}


template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const coupledFvPatchField<Type>& ptf
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(ptf.patch())),
    fvPatchField<Type>(ptf)
{}


template<class Type>
Foam::coupledFvPatchField<Type>::coupledFvPatchField
(
    const coupledFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(ptf.patch())),
    fvPatchField<Type>(ptf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::gpuField<Type> > Foam::coupledFvPatchField<Type>::snGrad
(
    const scalargpuField& deltaCoeffs
) const
{
    return
        deltaCoeffs
       *(this->patchNeighbourField() - this->patchInternalField());
}


template<class Type>
void Foam::coupledFvPatchField<Type>::initEvaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }
}


template<class Type>
void Foam::coupledFvPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    gpuField<Type>::operator=
    (
        this->patch().weights()*this->patchInternalField()
      + (1.0 - this->patch().weights())*this->patchNeighbourField()
    );

    fvPatchField<Type>::evaluate();
}


template<class Type>
Foam::tmp<Foam::gpuField<Type> >
Foam::coupledFvPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalargpuField>& w
) const
{
    return Type(pTraits<Type>::one)*w;
}


template<class Type>
Foam::tmp<Foam::gpuField<Type> >
Foam::coupledFvPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalargpuField>& w
) const
{
    return Type(pTraits<Type>::one)*(1.0 - w);
}


template<class Type>
Foam::tmp<Foam::gpuField<Type> >
Foam::coupledFvPatchField<Type>::gradientInternalCoeffs
(
    const scalargpuField& deltaCoeffs
) const
{
    return -Type(pTraits<Type>::one)*deltaCoeffs;
}


template<class Type>
Foam::tmp<Foam::gpuField<Type> >
Foam::coupledFvPatchField<Type>::gradientInternalCoeffs() const
{
    notImplemented("coupledFvPatchField<Type>::gradientInternalCoeffs()");
    return -Type(pTraits<Type>::one)*this->patch().deltaCoeffs();
}

template<class Type>
Foam::tmp<Foam::gpuField<Type> >
Foam::coupledFvPatchField<Type>::gradientBoundaryCoeffs
(
    const scalargpuField& deltaCoeffs
) const
{
    return -this->gradientInternalCoeffs(deltaCoeffs);
}

template<class Type>
Foam::tmp<Foam::gpuField<Type> >
Foam::coupledFvPatchField<Type>::gradientBoundaryCoeffs() const
{
    notImplemented("coupledFvPatchField<Type>::gradientBoundaryCoeffs()");
    return -this->gradientInternalCoeffs();
}


template<class Type>
void Foam::coupledFvPatchField<Type>::updateInterfaceMatrix
(
    scalargpuField& result,
    const scalargpuField& coeffs,
    const scalargpuField& pnf,
    const bool negate
) const
{
    if(negate)
        updateInterfaceMatrix<true>(result, coeffs, pnf);
    else
        updateInterfaceMatrix<false>(result, coeffs, pnf);
}


template<class Type>
template<bool negate>
void Foam::coupledFvPatchField<Type>::updateInterfaceMatrix
(
    scalargpuField& result,
    const scalargpuField& coeffs,
    const scalargpuField& pnf
) const
{
    // Multiply the field by coefficients and add into the result
    matrixPatchOperation
    (
        this->patch().index(),
        result,
        this->patch().boundaryMesh().mesh().lduAddr(),
        matrixInterfaceFunctor<scalar,negate>
        (
            coeffs.data(),
            pnf.data()
        )
    );
}


template<class Type>
void Foam::coupledFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);
    this->writeEntry("value", os);
}


// ************************************************************************* //
