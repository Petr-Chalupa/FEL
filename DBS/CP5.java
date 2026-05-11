import entities.Employee;
import entities.Issuperior;
import entities.Package;
import jakarta.persistence.*;
import java.util.List;

public class CP5 {
    public static void main(String[] args) {
        EntityManagerFactory emf = Persistence.createEntityManagerFactory("default");
        EntityManager em = emf.createEntityManager();

        EntityTransaction tx = em.getTransaction();
        tx.begin();

        // INSERT
        Query query_e1 = em.createQuery("SELECT e FROM Employee e WHERE e.phoneNumber = '700000139'");
        Query query_e2 = em.createQuery("SELECT e FROM Employee e WHERE e.phoneNumber = '700000128'");
        Employee e1 = (Employee) query_e1.getResultList().get(0);
        Employee e2 = (Employee) query_e2.getResultList().get(0);
        Issuperior issuperior = new Issuperior();
        issuperior.setEmployee(e1);
        issuperior.setSuperior(e2);
        em.persist(issuperior);

        // UPDATE
        Query query = em.createQuery("SELECT p FROM Package p WHERE p.id = 1");
        Package pkg1 = (Package) query.getResultList().get(0);
        pkg1.setPkgWeight(7777777);
        em.persist(pkg1);

        // DELETE
        Query q_i = em.createQuery("SELECT i FROM Issuperior i WHERE i.employee.phoneNumber = '700000139'");
        Issuperior is = (Issuperior) q_i.getResultList().get(0);
        em.remove(is);

        // SELECT
        Query q = em.createQuery("SELECT i from Issuperior i");
        List<Issuperior> islist = q.getResultList();
        for (Issuperior iss : islist) {
            System.out.println("Is superior: " + iss.getSuperior().getPhoneNumber() + " - " + iss.getEmployee().getPhoneNumber());
        }

        tx.commit();
    }
}
