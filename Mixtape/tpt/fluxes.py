# This file is part of MSMBuilder.
#
# Copyright 2011 Stanford University
#
# MSMBuilder is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

"""
Functions for performing Transition Path Theory calculations. 

Contributions from Kyle Beauchamp, Robert McGibbon, Vince Voelz,
Christian Schwantes, and TJ Lane.

These are the canonical references for TPT. Note that TPT is really a
specialization of ideas very framiliar to the mathematical study of Markov
chains, and there are many books, manuscripts in the mathematical literature
that cover the same concepts.

References
----------
.. [1] Metzner, P., Schutte, C. & Vanden-Eijnden, E. Transition path theory
       for Markov jump processes. Multiscale Model. Simul. 7, 1192-1219
       (2009).
.. [2] Berezhkovskii, A., Hummer, G. & Szabo, A. Reactive flux and folding 
       pathways in network models of coarse-grained protein dynamics. J. 
       Chem. Phys. 130, 205102 (2009).
"""
from __future__ import print_function, division, absolute_import
import numpy as np
import scipy.sparse

import itertools
import copy

import logging
logger = logging.getLogger(__name__)

# turn on debugging printout
# logger.setLogLevel(logging.DEBUG)

###############################################################################
# Typechecking/Utility Functions
#
def calculate_fluxes(sources, sinks, msm, committors=None):
    """
    Compute the transition path theory flux matrix.

    Parameters
    ----------
    sources : array_like, int
        The set of unfolded/reactant states.
    sinks : array_like, int
        The set of folded/product states.
    msm : mixtape.MarkovStateModel
        MSM that has been fit to data. 
    committors : np.ndarray, optional
        The committors associated with `sources`, `sinks`, and `tprob`.
        If not provided, is calculated from scratch. If provided, `sources`
        and `sinks` are ignored.

    Returns
    ------
    flux_matrix : np.ndarray
        The flux matrix

    References
    ----------
    .. [1] Metzner, P., Schutte, C. & Vanden-Eijnden, E. Transition path theory 
           for Markov jump processes. Multiscale Model. Simul. 7, 1192-1219
           (2009).
    .. [2] Berezhkovskii, A., Hummer, G. & Szabo, A. Reactive flux and folding 
           pathways in network models of coarse-grained protein dynamics. J. 
           Chem. Phys. 130, 205102 (2009).
    """
    sources = np.array(sources).reshape((-1,))
    sinks = np.array(sinks).reshape((-1,))

    populations = msm.populations_
    tprob = msm.transmat_

    # check if we got the committors
    if committors is None:
        committors = calculate_committors(sources, sinks, tprob)

    n = tprob.shape[0]

    X = np.zeros((n, n))
    Y = np.zeros((n, n))
    X[(np.arange(n), np.arange(n))] = populations * (1.0 - committors)
    Y[(np.arange(n), np.arange(n))] = committors

    fluxes = np.dot(np.dot(X, tprob), Y)
    fluxes[(np.arange(n), np.arange(n))] = np.zeros(n)

    return fluxes


def calculate_net_fluxes(sources, sinks, msm, committors=None):
    """
    Computes the transition path theory net flux matrix.

    Parameters
    ----------
    sources : array_like, int
        The set of unfolded/reactant states.
    sinks : array_like, int
        The set of folded/product states.
    msm : mixtape.MarkovStateModel
        MSM fit to data.
    committors : np.ndarray, optional
        The committors associated with `sources`, `sinks`, and `tprob`.
        If not provided, is calculated from scratch. If provided, `sources`
        and `sinks` are ignored.

    Returns
    ------
    net_flux : np.ndarray
        The net flux matrix

    References
    ----------
    .. [1] Metzner, P., Schutte, C. & Vanden-Eijnden, E. Transition path theory 
           for Markov jump processes. Multiscale Model. Simul. 7, 1192-1219
           (2009).
    .. [2] Berezhkovskii, A., Hummer, G. & Szabo, A. Reactive flux and folding 
           pathways in network models of coarse-grained protein dynamics. J. 
           Chem. Phys. 130, 205102 (2009).
    """

    flux_matrix = calculate_fluxes(sources, sinks, msm, committors=committors)

    net_flux = flux_matrix - flux_matrix.T
    net_flux[np.where(net_flux < 0)] = 0.0

    # Old Code:
    #for k in range(len(ind[0])):
    #    i, j = ind[0][k], ind[1][k]
    #    forward = flux[i, j]
    #    reverse = flux[j, i]
    #    net_flux[i, j] = max(0, forward - reverse)

    return net_flux
