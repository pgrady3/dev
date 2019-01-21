function [ deriv ] = splineDeriv( y, nDeriv, splineOrder)
%splineDeriv takes the derivative of a spline
%   Input - spline
%         - nDeriv (optional)
%         - splineOrder (optional)
%   Output - derivative in same structure spline
    
    if nargin<3
        splineOrder = 3;
    end
    if nargin<2
        nDeriv = 1;
    end
    
    deriv = y;

    % Derivative of Piecewise Spline Poly
    D = diag(splineOrder:-1:1,1);
    deriv.p.coefs = y.p.coefs*D^nDeriv;

end

