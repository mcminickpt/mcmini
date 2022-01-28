# DPOR
Implementation of DPOR algorithm


## DPOR in context with Dining Philosopher
2. let s = current node in depth-first-search
3. for all Philosophers p {
4.   if(i = last phil from set of  Philosopher got into fight with 
            other Philosopher to get the fork) {
5.        look at all time stamps b/w i and present, look at time
          stamp j with philosopher q, such that transition j with
          philosopher q must happens before.
          set E = set of all Philo who get into fight but are not
          depentend or connected with the above set of philospher.

6.        if E!=NULL select any thread from the set E and 
                     backtrack.
7.        else Backtrack for all transitions from i.   
                   
        }
}
