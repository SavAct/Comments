# Your favorite news platform or youtube channel doesn't have a comments section?
SavAct's comment section is a way out.

## Immutable comment section for all websites
This smart contract allows comments to be linked to web pages. The comments are immutable stored on blockchain.

## How it works
Someone creats a comment section for a link. Comments are free, but likes and dislikes are measured in system token amounts. For offering the RAM the amount for likes goes to the creator. The dislikes are withdrown to all token owners. 
As a website owner, you should know your URLs at first. Therefore, you can create the comment section as first, otherwise other peaple may do it and get the payments.

## Current Features
- No need of further RAM for each new comment. Only the reference to the current last comment is stored on RAM. All comments are connected in a chain within the chain of the blockchain.
- The first 12 characters of a domain are used as scope. 
- Conversion of all allowed characters in ACE strings to base 40 numbers to wrap them in one uint64_t type.
- The beginning characters "https<span>://</span>", "http<span>://</span>", "w<span>ww.</span>" and "m." of links are not distinguished.
- The primary key is generated by a hash to distribute entries evenly.
- Count all comments, without comments to a comment.  
- Count payments as likes or dislikes.
- Send payment amounts of dislikes to token owners.
- Send payment amounts of likes to creators. It deducts a fee that flows to the token owners. The fee avoids the creator to likes themself for an infinite amount of times.
- Creator of a comment section is allowed to mark unethical users. This information can be used to warn visitors before a marked comment is displayed.
