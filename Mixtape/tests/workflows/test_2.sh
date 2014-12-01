msmb AlanineDipeptide
msmb AtomIndices --out atom_indices.txt \
     -p ~/mixtape_data/alanine_dipeptide/ala2.pdb \
     -d --heavy
msmb AtomPairsFeaturizer --out atom_pairs/ \
    --trjs '~/mixtape_data/alanine_dipeptide/*.dcd' \
    --pair_indices atom_indices.dat \
    --top ~/mixtape_data/alanine_dipeptide/ala2.pdb
msmb MiniBatchKMeans --n_clusters 100 \
    --batch_size 1000 \
    --inp atom_pairs \
    --transformed kmedoids_centers.h5
msmb MarkovStateModel --inp kmedoids_centers.h5 \
    --out /dev/null

msmb tICA --inp atom_pairs/ --transformed atom_pairs_tica.h5 \
    --n_components 4 \
    --gamma 0 \
    --weighted_transform \
    --lag_time 2
msmb GaussianFusionHMM --inp atom_pairs_tica.h5 \
    --out hmm.pkl \
    --n_states 2 \
    --n_features 4
