/******************************************************************************
 * MATH FUNCTIONS 
 ******************************************************************************/
uint64_t math_factorial(uint64_t num) 
{
        uint64_t answer = 1;
  
        while (num > 1) {
                answer *= num;
                num--;
        }
        
        return answer;
}

int math_nchoosek(int n, int k)
{
        return (n==0 | k==0) ? 1 : (n*nchoosek(n-1,k-1))/k;
}
